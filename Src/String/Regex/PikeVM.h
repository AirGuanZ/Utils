#pragma once

#include <limits>
#include <type_traits>
#include <vector>

#include "../../Alloc/FixedSizedArena.h"
#include "../../Misc/Common.h"
#include "../String.h"

AGZ_NS_BEG(AGZ::RegexImpl::Pike)

// Submatching recorder
// Each thread hold a SaveSlots to record its matching track. When a thread
// meets a 'Save' instruction, it saves the current character location into a
// instruction-specified slot. Whenever a thread matches successfully, its
// saving slots gives all matched ranges.
class SaveSlots
{
    struct SaveSlotsStorage
    {
        size_t refs;
        size_t slots[1];
    };

    size_t slotCount_;
    SaveSlotsStorage *storage_;
    FixedSizedArena<> &arena_;

public:

    using Self = SaveSlots;

    static size_t AllocSize(size_t slotCount)
    {
        return sizeof(SaveSlotsStorage)
             + slotCount * sizeof(size_t)
             - sizeof(size_t);
    }

    SaveSlots(size_t slotCount, FixedSizedArena<> &arena)
        : slotCount_(slotCount), arena_(arena)
    {
        storage_ = arena_.Malloc<SaveSlotsStorage>();
        storage_->refs = 1;
        for(size_t i = 0; i < slotCount_; ++i)
            storage_->slots[i] = std::numeric_limits<size_t>::max();
    }

    SaveSlots(const Self &copyFrom)
        : slotCount_(copyFrom.slotCount_),
          storage_(copyFrom.storage_),
          arena_(copyFrom.arena_)
    {
        ++storage_->refs;
    }

    SaveSlots(Self &&moveFrom) noexcept
        : slotCount_(moveFrom.slotCount_),
          storage_(moveFrom.storage_),
          arena_(moveFrom.arena_)
    {
        moveFrom.storage_ = nullptr;
    }

    ~SaveSlots()
    {
        if(storage_ && !--storage_->refs)
            arena_.Free(storage_);
    }

    void Set(size_t slot, size_t value)
    {
        AGZ_ASSERT(slot < slotCount_ && storage_);

        // Copy-On-Write Strategy
        if(storage_->refs > 1)
        {
            auto newSto = arena_.Malloc<SaveSlotsStorage>();
            newSto->refs = 1;
            for(size_t i = 0; i < slotCount_; ++i)
                newSto->slots[i] = storage_->slots[i];

            --storage_->refs;
            storage_ = newSto;
        }
        storage_->slots[slot] = value;
    }

    size_t Get(size_t idx) const
    {
        AGZ_ASSERT(idx < slotCount_ && storage_);
        return storage_->slots[idx];
    }
};

// VM Instruction type
enum class InstOpCode
{
    Char,   // Match single character
    Jump,   // Unconditional Jump
    Branch, // Branch into two execution threads
    Save,   // Save current data pointer to given slot
    Match   // Report a successful matching
};

template<typename CP>
struct Instruction
{
    static_assert(std::is_trivially_constructible_v<CP>);

    InstOpCode op;

    // Instruction arguments
    union
    {
        CP cp;                            // For Char
        const Instruction *jumpDest;      // For Jump
        const Instruction *branchDest[2]; // For Branch
        size_t saveSlot;                  // For Save
    };

    // Character index when last thread running at this instruction
    // was added to ready list. Used to avoid more than one threads with the
    // same execution location to be added into ready list within one step.
    mutable size_t lastStep;
};

template<typename CS>
struct Thread
{
    Thread(const Instruction<typename CS::CodePoint> *pc,
           SaveSlots &&saveSlots)
        : pc(pc), saveSlots(std::move(saveSlots))
    {
        
    }

    const Instruction<typename CS::CodePoint> *pc;
    SaveSlots saveSlots;
};

template<typename CS>
class PikeCompiler
{
public:

    using CP    = typename CS::CodePoint;
    using Inst  = Instruction<CP>;
    using Insts = std::vector<Inst>;
    using It    = typename CS::Iterator;

    bool inSub_ = false;
    const StringView<CS> *src_ = nullptr;

    It cur_, end_;

    Insts *insts_;

private:

    /* Grammar
        Regex  := Cat Cat ... Cat
        Cat    := [Factor Factor ... Factor] | Factor | $Cat
        Factor := Factor* | Factor+ | Factor? | Core
        Core   := Char | (Regex)
    */
    size_t ParseRegex()
    {
        size_t ret = 0, dret;
        while(ParseCat(&dret))
            ret += dret;
        return ret;
    }

    bool ParseCat(size_t *slotCount)
    {
        // TODO
        *slotCount = 0;
        return true;
    }

public:

    void Compile(const StringView<CS> &src,
                 Insts &insts, size_t *slotCount)
    {
        AGZ_ASSERT(slotCount);

        auto cpSeq = src.CodePoints();

        inSub_     = false;
        src_       = &src;
        cur_       = cpSeq.begin();
        end_       = cpSeq.end();
        *slotCount = ParseRegex();

        if(cur_ != end_)
            throw ArgumentException("Invalid regular expression");
    }
};

// See https://swtch.com/~rsc/regexp/regexp2.html
template<typename CS>
class PikeMachine
{
    using CodeUnit  = typename CS::CodeUnit;
    using CodePoint = typename CS::CodePoint;

    using Arena = FixedSizedArena<>;

    std::vector<Instruction<typename CS::CodePoint>> prog_;
    String<CS> regex_;

    static Thread<CS> *NewThread(
        Arena &arena,
        const Instruction<CodePoint> *pc,
        SaveSlots &&slots)
    {
        Thread<CS> *ret = arena.Malloc<Thread<CS>>();
        return new(ret) Thread<CS>(pc, std::move(slots));
    }

    static void FreeThread(Arena &arena, Thread<CS> *th)
    {
        th->~Thread();
        arena.Free(th);
    }

    static std::pair<bool, std::vector<std::pair<size_t, size_t>>> Run(
        const std::vector<Instruction<CodePoint>> prog,
        size_t slotCount,
        const StringView<CS> &dst)
    {
        AGZ_ASSERT(slotCount % 2 == 0);

        Arena threadArena(sizeof(Thread<CS>), 32 * sizeof(Thread<CS>));
        Arena slotsArena(SaveSlots::AllocSize(slotCount),
                         32 * SaveSlots::AllocSize(slotCount));

        std::vector<Thread<CS>*> rdyThds = {
            NewThread(threadArena, &prog[0],
                      SaveSlots(slotCount, slotsArena))
        };
        std::vector<Thread<CS>*> newThds;

        for(auto &inst : prog)
            inst.lastStep = std::numeric_limits<size_t>::max();

        size_t step = 0;
        auto cpSeq = dst.CodePoints();
        auto end = cpSeq.end();
        for(auto it = cpSeq.begin(); it != end; ++it)
        {
            CodePoint cp = *it;
            if(rdyThds.empty())
                break;
            for(size_t i = 0; i < rdyThds.size(); ++i)
            {
                Thread<CS> *th = rdyThds[i];
                auto pc = th->pc;

                switch(pc->op)
                {
                case InstOpCode::Jump:
                    if(pc->jumpDest->lastStep != step)
                    {
                        th->pc = pc->jumpDest;
                        pc->jumpDest->lastStep = i;
                        rdyThds.push_back(th);
                    }
                    break;
                case InstOpCode::Char:
                    if(pc->cp == cp && (pc + 1)->lastStep != step)
                    {
                        ++th->pc;
                        th->pc->lastStep = step;
                        newThds.push_back(th);
                    }
                    else
                        FreeThread(threadArena, th);
                    break;
                case InstOpCode::Branch:
                    // IMPROVE: Save a copy construction when
                    //      dest[0] != step and dest[1] == step
                    if(pc->branchDest[0]->lastStep != step)
                    {
                        pc->branchDest[0]->lastStep = step;
                        rdyThds.push_back(NewThread(threadArena,
                            pc->branchDest[0], SaveSlots(th->saveSlots)));
                    }
                    if(pc->branchDest[1]->lastStep != step)
                    {
                        th->pc = pc->branchDest[1];
                        th->pc->lastStep = step;
                        rdyThds.push_back(th);
                    }
                    else
                        FreeThread(threadArena, th);
                    break;
                case InstOpCode::Save:
                    if((pc + 1)->lastStep != step)
                    {
                        th->saveSlots.Set(pc->saveSlot, cpSeq.CodeUnitIndex(it));
                        ++th->pc;
                        th->pc->lastStep = step;
                        rdyThds.push_back(th);
                    }
                    break;
                default:
                    Unreachable();
                }
            }

            rdyThds.swap(newThds);
            newThds.clear();
            ++step;
        }

        // Any thread matched?
        for(auto th : rdyThds)
        {
            if(th->pc->op == InstOpCode::Match)
            {
                std::vector<std::pair<size_t, size_t>> slots(slotCount / 2);
                for(size_t i = 0; i < slots.size(); ++i)
                {
                    slots[i].first  = th->saveSlots.Get(i << 2);
                    slots[i].second = th->saveSlots.Get((i << 2) + 1);
                }
                return { true, std::move(slots) };
            }
        }

        return { false, { } };
    }

    // TODO: Compile regex to VM instructions

public:

    static constexpr bool SupportSubmatching = true;

    // TODO: Implement RegexEngine concept (see Regex.h)
};

AGZ_NS_END(AGZ::RegexImpl::Pike)
