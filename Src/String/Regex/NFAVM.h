#pragma once

#include <vector>

#include "../../Alloc/FixedSizedArena.h"
#include "../../Misc/Common.h"
#include "../String.h"

AGZ_NS_BEG(AGZ::RegexImpl::NFAVM)

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

    Self &operator=(const Self &) = delete;

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

struct Inst
{
    enum { Char, Jump, Branch, Alter, Save, Match } op;
    union
    {
        char32_t codePoint;
        int jumpDest;
        struct
        {
            int branchX;
            int branchY;
        };
        std::vector<int> *alterDest;
        unsigned int saveSlot;
    };
    mutable size_t lastStep;
};

inline Inst MakeChar(char32_t ch) { Inst r = { Inst::Char }; r.codePoint = ch; return r; }
inline Inst MakeJump(int dest) { Inst r = { Inst::Jump }; r.jumpDest = dest; return r; }
inline Inst MakeBranch(int x, int y) { Inst r = { Inst::Branch }; r.branchX = x; r.branchY = y; return r; }
inline Inst MakeAlter(std::vector<int> *alterDest) { Inst r = { Inst::Alter }; r.alterDest = alterDest; return r; }
inline Inst MakeSave(unsigned int slot) { Inst r = { Inst::Save }; r.saveSlot = slot; return r; }
inline Inst MakeMatch() { Inst r = { Inst::Match }; return r; }

using Program = std::vector<Inst>;

template<typename CS>
class NFAVMCompiler
{
public:

    void Compile(const StringView<CS> &regex, Program *prog, size_t *slotCount);

private:

    using CP = typename CS::CodePoint;
    using It = typename CS::Iterator;

    It cur_, end_;
    size_t nextSaveSlot_;
    bool inSubmatching_;
};

AGZ_NS_END(AGZ::RegexImpl::NFAVM)
