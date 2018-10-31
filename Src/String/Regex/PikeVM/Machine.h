#pragma once

#include <limits>
#include <optional>
#include <utility>

#include "../../../Alloc/FixedSizedArena.h"
#include "../../../Misc/Common.h"
#include "Syntax.h"
#include "Backend.h"

AGZ_NS_BEG(AGZ::PikeVM)

class SaveSlots
{
    struct SaveSlotsStorage
    {
        mutable size_t refs;
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
        storage_ = static_cast<SaveSlotsStorage*>(arena_.Alloc());
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
            auto newSto = static_cast<SaveSlotsStorage*>(arena_.Alloc());
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
struct Thread
{
    Thread(Inst<CP> *pc, SaveSlots &&saveSlots,
           bool charExprReg, size_t startIdx)
        : pc(pc), saveSlots(std::move(saveSlots)),
          charExprReg(charExprReg), startIdx(startIdx)
    {

    }

    Inst<CP> *pc;
    SaveSlots saveSlots;
    bool charExprReg;
    size_t startIdx;
};

template<typename CS>
class Machine
{
public:

    using CU = typename CS::CodeUnit;
    using CP = typename CS::CodePoint;

    using Interval = std::pair<size_t, size_t>;

    explicit Machine(const StringView<CS> &regex)
        : slotCount_(0), regex_(regex)
    {
        
    }

    std::optional<std::vector<size_t>>
        Match(const StringView<CS> &dst) const
    {
        if(!prog_.Available())
            Compile();
        auto ret = Run<true, true>(dst);
        return ret.has_value() ? std::make_optional(
                                    std::move(ret.value().second))
                               : std::nullopt;
    }

    std::optional<std::pair<std::pair<size_t, size_t>,
                            std::vector<size_t>>>
        Search(const StringView<CS> &dst) const
    {
        if(!prog_.Available())
            Compile();
        return Run<false, false>(dst);
    }

    std::optional<std::pair<std::pair<size_t, size_t>,
                  std::vector<size_t>>>
        SearchPrefix(const StringView<CS> &dst) const
    {
        if(!prog_.Available())
            Compile();
        return Run<true, false>(dst);
    }

    std::optional<std::pair<std::pair<size_t, size_t>,
                  std::vector<size_t>>>
        SearchSuffix(const StringView<CS> &dst) const
    {
        if(!prog_.Available())
            Compile();
        return Run<false, true>(dst);
    }

private:

    using It = typename CS::Iterator;
    using CPR = StrImpl::CodePointRange<CS>;

    mutable Program<CP> prog_;
    mutable size_t slotCount_;
    mutable String<CS> regex_;

    struct MatchState
    {
        CPR *cpr;
        It cur;

        size_t matchedStart;
        size_t matchedEnd;
        std::optional<SaveSlots> matchedSaveSlots;

        uint32_t cpIdx;
    };

    void Compile() const
    {
        prog_ = Backend<CS>().Generate(
            Parser<CS>().Parse(regex_), &slotCount_);
        regex_ = String<CS>();
        AGZ_ASSERT(prog_.Available() && prog_.Full());
    }

    void AddThread(
        MatchState &state,
        std::vector<Thread<CP>> &thds,
        Inst<CP> *pc, CP cp,
        SaveSlots &&saves, bool reg,
        size_t startIdx) const
    {
        if(pc->lastStep == state.cpIdx)
            return;

        switch(pc->type)
        {
        case InstType::Begin:
            if(state.cpr->begin() != state.cur)
                return;
            AddThread(
                state, thds,
                pc + 1, cp, std::move(saves), reg, startIdx);
            break;

        case InstType::End:
            if(state.cpr->end() != state.cur)
                return;
            AddThread(
                state, thds,
                pc + 1, cp, std::move(saves), reg, startIdx);
            break;

        case InstType::Save:
            saves.Set(pc->dataSave.slot,
                      state.cpr->CodeUnitIndex(state.cur));
            AddThread(
                state, thds,
                pc + 1, cp, std::move(saves), reg, startIdx);
            break;

        case InstType::Alter:
        {
            auto alterDests = prog_.GetRelativeOffsetArray(
                prog_.GetInstIndex(pc));
            for(uint32_t i = 0; i < pc->dataAlter.count; ++i)
            {
                AddThread(
                    state, thds,
                    pc + alterDests[i], cp,
                    SaveSlots(saves), reg, startIdx);
            }
            break;
        }

        case InstType::Jump:
            AddThread(
                state, thds,
                pc + pc->dataJump.offset, cp,
                std::move(saves), reg, startIdx);
            break;

        case InstType::Branch:
            AddThread(
                state, thds,
                pc + pc->dataBranch.dest[0], cp,
                SaveSlots(saves), reg, startIdx);
            AddThread(
                state, thds,
                pc + pc->dataBranch.dest[1], cp,
                std::move(saves), reg, startIdx);
            break;

        case InstType::CharExprSingle:
            AddThread(
                state, thds,
                pc + 1, cp,
                std::move(saves),
                cp == pc->dataCharExprSingle.codePoint,
                startIdx);
            break;

        case InstType::CharExprAny:
            AddThread(
                state, thds,
                pc + 1, cp,
                std::move(saves), true, startIdx);
            break;

        case InstType::CharExprRange:
            AddThread(
                state, thds,
                pc + 1, cp,
                std::move(saves),
                pc->dataCharExprRange.fst <= cp &&
                cp <= pc->dataCharExprRange.lst,
                startIdx);
            break;

        case InstType::CharExprDecDigit:
            AddThread(
                state, thds,
                pc + 1, cp,
                std::move(saves),
                StrAlgo::IsUnicodeDigit(cp),
                startIdx);
            break;

        case InstType::CharExprHexDigit:
            AddThread(
                state, thds,
                pc + 1, cp,
                std::move(saves),
                StrAlgo::IsUnicodeHexDigit(cp),
                startIdx);
            break;

        case InstType::CharExprAlpha:
            AddThread(
                state, thds,
                pc + 1, cp,
                std::move(saves),
                StrAlgo::IsUnicodeAlpha(cp),
                startIdx);
            break;

        case InstType::CharExprWordChar:
            AddThread(
                state, thds,
                pc + 1, cp,
                std::move(saves),
                StrAlgo::IsUnicodeAlnum(cp) || cp == '_',
                startIdx);
            break;

        case InstType::CharExprWhitespace:
            AddThread(
                state, thds,
                pc + 1, cp,
                std::move(saves),
                StrAlgo::IsUnicodeWhitespace(cp),
                startIdx);
            break;
            
        case InstType::CharExprITSTAJ:
            AddThread(
                state, thds,
                pc + (reg ? pc->dataITSTAJ.offset : 1), cp,
                std::move(saves),
                reg,
                startIdx);
            break;

        case InstType::CharExprIFSFAJ:
            AddThread(
                state, thds,
                pc + (!reg ? pc->dataIFSFAJ.offset : 1), cp,
                std::move(saves),
                reg,
                startIdx);
            break;

        case InstType::CharExprSetTrue:
            AddThread(
                state, thds,
                pc + 1, cp,
                std::move(saves),
                true,
                startIdx);
            break;

        case InstType::CharExprSetFalse:
            AddThread(
                state, thds,
                pc + 1, cp,
                std::move(saves),
                false,
                startIdx);
            break;

        case InstType::CharExprNot:
            AddThread(
                state, thds,
                pc + 1, cp,
                std::move(saves),
                !reg,
                startIdx);
            break;

        default:
            thds.push_back(Thread<CP>(
                pc, std::move(saves), reg, startIdx));
            break;
        }
    }

    void AddThreadWithPC(
        MatchState &state,
        std::vector<Thread<CP>> &thds,
        Inst<CP> *pc,
        CP cp, bool reg,
        Thread<CP> *oriTh) const
    {
        auto oldCur = state.cur;
        ++state.cur;
        AddThread(
            state,
            thds,
            pc, cp,
            std::move(oriTh->saveSlots),
            reg, oriTh->startIdx);
        state.cur = oldCur;
    }

    static CP NextCP(MatchState &state)
    {
        auto it = state.cur;
        if(++it == state.cpr->end())
            return std::numeric_limits<CP>::max();
        return *it;
    }

#define STEP_TO_NEXT_PC() \
    do { \
        AddThreadWithPC( \
            state, newThds, pc + 1, \
            NextCP(state), th->charExprReg, th); \
    } while(0)

    template<bool AnchorBegin, bool AnchorEnd>
    std::optional<std::pair<Interval, std::vector<size_t>>>
        Run(const StringView<CS> &str) const
    {
        AGZ_ASSERT(prog_.Available());

        size_t saveAllocSize = SaveSlots::AllocSize(slotCount_);
        FixedSizedArena<> saveSlotsArena(saveAllocSize);

        prog_.ReinitLastSteps();
        std::vector<Thread<CP>> rdyThds, newThds;
        rdyThds.reserve(prog_.Size());
        newThds.reserve(prog_.Size());

        CPR cpr = str.CodePoints();
        MatchState state;
        state.cur = cpr.begin();
        state.cpr = &cpr;
        state.matchedStart = 0;
        state.matchedEnd = 0;
        state.matchedSaveSlots.reset();
        state.cpIdx = 0;

        if constexpr(AnchorBegin)
        {
            if(!str.Empty())
            {
                AddThread(
                    state, rdyThds, &prog_.GetInst(0), *state.cur,
                    SaveSlots(slotCount_, saveSlotsArena),
                    true, 0);
            }
        }

        for(; state.cur != cpr.end(); ++state.cur, ++state.cpIdx)
        {
            CP cp = *state.cur;

            if constexpr(!AnchorBegin)
            {
                AddThread(
                    state, rdyThds, &prog_.GetInst(0),
                    cp, SaveSlots(slotCount_, saveSlotsArena),
                    true, state.cpIdx);
            }
            else
            {
                if(rdyThds.empty())
                    break;
            }

            for(size_t i = 0; i < rdyThds.size(); ++i)
            {
                Thread<CP> *th = &rdyThds[i];
                Inst<CP> *pc = th->pc;

                switch(pc->type)
                {
                case InstType::CharSingle:
                    if(pc->dataCharSingle.codePoint == cp)
                        STEP_TO_NEXT_PC();
                    break;

                case InstType::CharAny:
                    STEP_TO_NEXT_PC();
                    break;

                case InstType::CharRange:
                    if(pc->dataCharRange.fst <= cp &&
                       cp <= pc->dataCharRange.lst)
                        STEP_TO_NEXT_PC();
                    break;

                case InstType::CharDecDigit:
                    if(StrAlgo::IsUnicodeDigit(cp))
                        STEP_TO_NEXT_PC();
                    break;

                case InstType::CharHexDigit:
                    if(StrAlgo::IsUnicodeHexDigit(cp))
                        STEP_TO_NEXT_PC();
                    break;

                case InstType::CharAlpha:
                    if(StrAlgo::IsUnicodeAlpha(cp))
                        STEP_TO_NEXT_PC();
                    break;

                case InstType::CharWordChar:
                    if(StrAlgo::IsUnicodeAlnum(cp) || cp == '_')
                        STEP_TO_NEXT_PC();
                    break;

                case InstType::CharWhitespace:
                    if(StrAlgo::IsUnicodeWhitespace(cp))
                        STEP_TO_NEXT_PC();
                    break;

                case InstType::CharExprEnd:
                    if(th->charExprReg)
                        STEP_TO_NEXT_PC();
                    break;

                case InstType::Match:
                    if constexpr(!AnchorEnd)
                    {
                        state.matchedSaveSlots.emplace(
                            std::move(th->saveSlots));
                        state.matchedStart = th->startIdx;
                        state.matchedEnd = cpr.CodeUnitIndex(state.cur);
                        rdyThds.clear();
                    }
                    break;

                default:
                    Unreachable();
                }
            }

            rdyThds.swap(newThds);
            newThds.clear();
        }

        for(auto &th : rdyThds)
        {
            if(th.pc->type == InstType::Match)
            {
                state.matchedSaveSlots.emplace(std::move(th.saveSlots));
                state.matchedStart = th.startIdx;
                state.matchedEnd = str.Length();
                break;
            }
        }

        if(state.matchedSaveSlots)
        {
            std::vector<size_t> slots(slotCount_);
            for(size_t i = 0; i < slotCount_; ++i)
                slots[i] = state.matchedSaveSlots.value().Get(i);
            return std::make_pair(Interval{ state.matchedStart,
                                            state.matchedEnd },
                std::move(slots));
        }

        return std::nullopt;
    }

#undef STEP_TO_NEXT_PC
};

AGZ_NS_END(AGZ::PikeVM)
