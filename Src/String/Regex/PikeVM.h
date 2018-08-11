#pragma once

#include <iostream>

#include <limits>
#include <list>
#include <type_traits>
#include <utility>
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

template<typename CP>
struct Instruction;

template<typename CP>
using PC = typename std::list<Instruction<CP>>::iterator;

// VM Instruction type
enum class InstOpCode
{
    Char,   // Match single character
    Jump,   // Unconditional Jump
    Branch, // Branch into two threads
    Alter,  // Branch into many threads
    Save,   // Save current data pointer to given slot
    Match   // Report a successful matching
};

template<typename CP>
struct Instruction
{
    explicit Instruction(InstOpCode op)
        : op(op)
    {
        
    }

    Instruction(const Instruction<CP> &copyFrom)
    {
        op = copyFrom.op;
        lastStep = copyFrom.lastStep;
        switch(op)
        {
        case InstOpCode::Jump:
            jumpDest = copyFrom.jumpDest;
            break;
        case InstOpCode::Branch:
            branchDest[0] = copyFrom.branchDest[0];
            branchDest[1] = copyFrom.branchDest[1];
            break;
        case InstOpCode::Alter:
            alterDest = new std::vector<PC<CP>>(*copyFrom.alterDest);
            break;
        case InstOpCode::Char:
            cp = copyFrom.cp;
            break;
        case InstOpCode::Save:
            saveSlot = copyFrom.saveSlot;
            break;
        default:
            break;
        }
    }

    Instruction(Instruction<CP> &&moveFrom) noexcept
    {
        op = moveFrom.op;
        lastStep = moveFrom.lastStep;
        switch(op)
        {
        case InstOpCode::Jump:
            jumpDest = std::move(moveFrom.jumpDest);
            break;
        case InstOpCode::Branch:
            branchDest[0] = std::move(moveFrom.branchDest[0]);
            branchDest[1] = std::move(moveFrom.branchDest[1]);
            break;
        case InstOpCode::Alter:
            alterDest = moveFrom.alterDest;
            moveFrom.alterDest = nullptr;
            break;
        case InstOpCode::Char:
            cp = moveFrom.cp;
            break;
        case InstOpCode::Save:
            saveSlot = moveFrom.saveSlot;
            break;
        default:
            break;
        }
    }

    ~Instruction()
    {
        if(op == InstOpCode::Alter && alterDest)
            delete alterDest;
    }

    InstOpCode op;

    // Instruction arguments
    union
    {
        CP cp;                                // For Char
        size_t saveSlot;                      // For Save
        std::vector<PC<CP>> *alterDest;
    };
    PC<CP> jumpDest;                          // For Jump
    PC<CP> branchDest[2];                     // For Branch

    // Character index when last thread running at this instruction
    // was added to ready list. Used to avoid more than one threads with the
    // same execution location to be added into ready list within one step.
    mutable size_t lastStep;
};

template<typename CP>
Instruction<CP> MakeChar(CP cp)
{
    Instruction<CP> ret(InstOpCode::Char);
    ret.cp = cp;
    return ret;
}

template<typename CP>
Instruction<CP> MakeJump(PC<CP> dst)
{
    Instruction<CP> ret(InstOpCode::Jump);
    ret.jumpDest = dst;
    return ret;
}

template<typename CP>
Instruction<CP> MakeBranch(PC<CP> d0, PC<CP> d1)
{
    Instruction<CP> ret(InstOpCode::Branch);
    ret.branchDest[0] = d0; ret.branchDest[1] = d1;
    return ret;
}

template<typename CP>
Instruction<CP> MakeAlter(std::vector<PC<CP>> &&ds)
{
    Instruction<CP> ret(InstOpCode::Alter);
    ret.alterDest = new std::vector<PC<CP>>(std::move(ds));
    return ret;
}

template<typename CP>
Instruction<CP> MakeSave(size_t slot)
{
    Instruction<CP> ret(InstOpCode::Save);
    ret.saveSlot = slot;
    return ret;
}

template<typename CP>
Instruction<CP> MakeMatch()
{
    Instruction<CP> ret(InstOpCode::Match);
    return ret;
}

template<typename CS>
struct Thread
{
    Thread(PC<typename CS::CodePoint> pc,
           SaveSlots &&saveSlots)
        : pc(pc), saveSlots(std::move(saveSlots))
    {

    }

    PC<typename CS::CodePoint>  pc;
    SaveSlots saveSlots;
};

/* Grammars
    Regex  := Cat Cat ... Cat
    Cat    := Fac | $Cat
    Fac    := Fac* | Fac+ | Fac? | Core
    Core   := Char | (Regex) | [Fac Fac ... Fac]

   Rules
    A | B | C =>     Branch(L0, L1)
                 L0: Branch(L2, L3)
                 L1: Inst(A)
                     Jump(Out)
                 L2: Inst(B)
                     Jump(Out)
                 L3: Inst(C)

    AB => Inst(A)
          Inst(B)

    $A => Save(SlotPairIndex * 2)
          Inst(A)
          Save(SlotPairIndex * 2 + 1)

    A* => L0: Branch(L1, Out)
          L1: Inst(A)
              Jump(L0)

    A+ => L0: Inst(A)
              Branch(L0, Out)

    A? =>     Branch(L0, Out)
          L0: Inst(A)
*/
template<typename CS>
class PikeCompiler
{
public:

    void Compile(const StringView<CS> &expr,
                 std::list<Instruction<typename CS::CodePoint>> *insts,
                 size_t *slotCount)
    {
        AGZ_ASSERT(insts && slotCount);

        auto cpSeq = expr.CodePoints();
        cur_  = cpSeq.begin();
        end_  = cpSeq.end();

        nextSaveSlot_  = 0;
        inSubmatching_ = false;

        PartialResult result;
        CompileRegex(result);
        FillBP(result, Emit(result, MakeMatch<CP>()));

        if(cur_ != end_)
            Err();

        *insts     = std::move(result.insts);
        *slotCount = nextSaveSlot_;
    }

private:

    using CP        = typename CS::CodePoint;
    using Inst      = Instruction<CP>;
    using It        = typename CS::Iterator;

    template<typename T>
    using L = std::list<T>;

    It cur_, end_;
    size_t nextSaveSlot_ = 0;
    bool inSubmatching_  = false;

    bool IsEnd()     const   { return cur_ == end_;                          }
    CP   CurCh()     const   { AGZ_ASSERT(!IsEnd()); return *cur_;           }
    CP   CurAndAdv()         { CP rt = CurCh(); Advance(); return rt;        }
    bool Match(CP cp)        { return !IsEnd() && CurCh() == cp;             }
    void Advance()           { AGZ_ASSERT(!IsEnd()); ++cur_;                 }
    bool AdvanceIf(CP cp)    { return Match(cp) ? (Advance(), true) : false; }
    void AdvanceOrErr(CP cp) { if(!AdvanceIf(cp)) Err();                     }

    [[noreturn]] static void Err() { throw ArgumentException("Invalid regex"); }
    void ErrIfEnd() const { if(IsEnd()) Err(); }

    struct PartialResult
    {
        L<Inst> insts;
        L<PC<CP>> bps;
    };

    static PC<CP> Emit(PartialResult &out, const Inst &inst)
    {
        out.insts.push_back(inst);
        auto ret = out.insts.end();
        return --ret;
    }

    static PC<CP> EmitFront(PartialResult &out, const Inst &inst)
    {
        out.insts.push_front(inst);
        return out.insts.begin();
    }

    void Concat(L<Inst> &lhs, L<Inst> &&rhs)
    {
        lhs.splice(lhs.end(), std::move(rhs));
    }

    void FillBP(PartialResult &out, PC<CP> value)
    {
        for(auto &p : out.bps)
        {
            if(p->op == InstOpCode::Jump)
                p->jumpDest = value;
            else
            {
                AGZ_ASSERT(p->op == InstOpCode::Branch);
                p->branchDest[1] = value;
            }
        }
        out.bps.clear();
    }

    bool CompileChar(PartialResult &out)
    {
        if(IsEnd())
            return false;
        CP cp = CurCh();

        // Special symbol
        switch(cp)
        {
        case '[':
        case ']':
        case '(':
        case ')':
        case '+':
        case '*':
        case '?':
        case '$':
            return false;
        default:
            break;
        }

        Advance();
        CP dst;

        // Escape
        if(cp == '\\')
        {
            ErrIfEnd();
            CP next = CurAndAdv();
            switch(next)
            {
            case 'a': dst = '\a'; break;
            case 'b': dst = '\b'; break;
            case 'n': dst = '\n'; break;
            case 'r': dst = '\r'; break;
            case 't': dst = '\t'; break;
            case '[':
            case ']':
            case '(':
            case ')':
            case '+':
            case '*':
            case '?':
            case '$':
            case '\\':
                dst = next; break;
            default:
                Err();
            }
        }
        else
            dst = cp;

        Emit(out, MakeChar(dst));
        return true;
    }

    bool CompileCore(PartialResult &out)
    {
        if(IsEnd())
            return false;

        if(AdvanceIf('('))
        {
            CompileRegex(out);
            AdvanceOrErr(')');
            return true;
        }

        /*
        A | B | C | D =>
        Alter(L0, L1, L2, L3)
        L0: Inst(A)
        Jump(Out)
        L1: Inst(B)
        Jump(Out)
        L2: Inst(C)
        Jump(Out)
        L3: Inst(D)
        */
        if(AdvanceIf('['))
        {
            L<PartialResult> facs;
            while(!AdvanceIf(']'))
            {
                facs.push_back(PartialResult());
                if(!CompileFac(facs.back()))
                    Err();
            }

            if(facs.empty())
                Err();

            if(facs.size() == 1)
            {
                out.insts = std::move(facs.front().insts);
                out.bps = std::move(facs.front().bps);
                return true;
            }

            for(auto &rt : facs)
                out.bps.splice(out.bps.end(), rt.bps);

            Concat(out.insts, std::move(facs.front().insts));
            std::vector<PC<CP>> alterDest = { out.insts.begin() };

            auto it = facs.begin();
            for(++it; it != facs.end(); ++it)
            {
                auto jump = Emit(out, MakeJump<CP>(PC<CP>()));
                out.bps.push_back(jump);

                alterDest.push_back(it->insts.begin());
                Concat(out.insts, std::move(it->insts));
            }

            EmitFront(out, MakeAlter<CP>(std::move(alterDest)));

            return true;
        }

        return CompileChar(out);
    }

    bool CompileFac(PartialResult &out)
    {
        if(!CompileCore(out))
            return false;

        for(;;)
        {
            /*
                A* => L0: Branch(L1, Out)
                      L1: Inst(A)
                          Jump(L0)
            */
            if(AdvanceIf('*'))
            {
                auto branch = EmitFront(
                        out, MakeBranch<CP>(out.insts.begin(), PC<CP>()));
                Emit(out, MakeJump<CP>(branch));
                FillBP(out, branch);
                out.bps.push_back(branch);
            }
            /*
                A+ => L0: Inst(A)
                          Branch(L0, Out)
            */
            else if(AdvanceIf('+'))
            {
                auto branch = Emit(
                        out, MakeBranch<CP>(out.insts.begin(), PC<CP>()));
                FillBP(out, branch);
                out.bps.push_back(branch);
            }
            /*
                A? =>     Branch(L0, Out)
                      L0: Inst(A)
            */
            else if(AdvanceIf('?'))
            {
                auto branch = EmitFront(
                        out, MakeBranch<CP>(out.insts.begin(), PC<CP>()));
                out.bps.push_back(branch);
            }
            else
                break;
        }

        return true;
    }

    bool CompileCat(PartialResult &out)
    {
        if(AdvanceIf('$'))
        {
            if(inSubmatching_)
                Err();
            inSubmatching_ = true;
            CompileCat(out);
            inSubmatching_ = false;

            EmitFront(out, MakeSave<CP>(nextSaveSlot_));
            FillBP(out, Emit(out, MakeSave<CP>(nextSaveSlot_ + 1)));
            nextSaveSlot_ += 2;

            return true;
        }

        return CompileFac(out);
    }

    void CompileRegex(PartialResult &out)
    {
        if(!CompileCat(out))
            return;
        PartialResult trt;
        while(CompileCat(trt))
        {
            FillBP(out, trt.insts.begin());
            Concat(out.insts, std::move(trt.insts));
            out.bps = std::move(trt.bps);

            AGZ_ASSERT(trt.insts.empty() && trt.bps.empty());
        }
    }
};

// See https://swtch.com/~rsc/regexp/regexp2.html
template<typename CS>
class PikeMachine
{
    using CodeUnit  = typename CS::CodeUnit;
    using CodePoint = typename CS::CodePoint;
    using CP = CodePoint;

    using Arena = FixedSizedArena<>;

    mutable std::list<Instruction<typename CS::CodePoint>> prog_;
    mutable size_t slotCount_;
    mutable String<CS> regex_;

    static Thread<CS> *NewThread(
        Arena &arena, PC<CP> pc, SaveSlots &&slots)
    {
        Thread<CS> *ret = arena.Malloc<Thread<CS>>();
        return new(ret) Thread<CS>(pc, std::move(slots));
    }

    static void FreeThread(Arena &arena, Thread<CS> *th)
    {
        th->~Thread();
        arena.Free(th);
    }

    std::pair<bool, std::vector<std::pair<size_t, size_t>>>
        MatchImpl(const StringView<CS> &dst) const
    {
        AGZ_ASSERT(slotCount_ % 2 == 0);

        Arena threadArena(sizeof(Thread<CS>), 32 * sizeof(Thread<CS>));
        Arena slotsArena(SaveSlots::AllocSize(slotCount_),
                         32 * SaveSlots::AllocSize(slotCount_));

        std::vector<Thread<CS>*> rdyThds = {
            NewThread(threadArena, prog_.begin(),
                      SaveSlots(slotCount_, slotsArena))
        };
        std::vector<Thread<CS>*> newThds;

        for(auto &inst : prog_)
            inst.lastStep = std::numeric_limits<size_t>::max();

        size_t step = 0;
        auto cpSeq = dst.CodePoints();
        auto end = cpSeq.end();
        for(auto it = cpSeq.begin(); ; ++it)
        {
            CodePoint cp;
            if(it != end)
                cp = *it;
            else
                cp = '\0';

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
                        th->pc->lastStep = i;
                        rdyThds.push_back(th);
                    }
                    else
                        FreeThread(threadArena, th);
                    break;

                case InstOpCode::Char:
                    if(pc->cp == cp && (++th->pc)->lastStep != step + 1)
                    {
                        th->pc->lastStep = step + 1;
                        newThds.push_back(th);
                    }
                    else
                        FreeThread(threadArena, th);
                    break;

                case InstOpCode::Branch:
                    // IMPROVE: Copy construction is unnecessary when
                    //              dest[0]->lastStep != step and
                    //              dest[1]->lastStep == step
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
                
                case InstOpCode::Alter:
                    // IMPROVE: Ditto
                    for(auto dest : *pc->alterDest)
                    {
                        if(dest->lastStep != step)
                        {
                            dest->lastStep = step;
                            rdyThds.push_back(NewThread(threadArena,
                                dest, SaveSlots(th->saveSlots)));
                        }
                    }
                    FreeThread(threadArena, th);
                    break;

                case InstOpCode::Save:
                    th->saveSlots.Set(pc->saveSlot, cpSeq.CodeUnitIndex(it));
                    if((++th->pc)->lastStep != step)
                    {
                        th->pc->lastStep = step;
                        rdyThds.push_back(th);
                    }
                    break;

                case InstOpCode::Match:
                    FreeThread(threadArena, th);
                    break;

                default:
                    Unreachable();
                }
            }

            if(it == end)
                break;

            rdyThds.swap(newThds);
            newThds.clear();
            ++step;
        }

        // Any thread matched?
        for(auto th : rdyThds)
        {
            if(th->pc->op == InstOpCode::Match)
            {
                std::vector<std::pair<size_t, size_t>> slots(slotCount_ / 2);
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

    void Compile() const
    {
        AGZ_ASSERT(prog_.empty());
        PikeCompiler<CS>().Compile(regex_, &prog_, &slotCount_);
        regex_ = String<CS>();
    }

public:

    static constexpr bool SupportSubmatching = true;

    explicit PikeMachine(const StringView<CS> &regex)
        : slotCount_(0), regex_(regex)
    {
        Compile();

        using namespace std;
        
        string names[] =
        {
            "Char", "Jump", "Branch", "Alter", "Save", "Match"
        };

        for(auto &ins : prog_)
        {
            cout << names[(int)ins.op] << endl;
        }
    }

    std::pair<bool, std::vector<std::pair<size_t, size_t>>>
    Match(const StringView<CS> &dst) const
    {
        if(prog_.empty())
            Compile();
        AGZ_ASSERT(prog_.size() && regex_.Empty());

        return MatchImpl(dst);
    }
};

AGZ_NS_END(AGZ::RegexImpl::Pike)
