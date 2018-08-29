#pragma once

#include <type_traits>

#include "../../../Alloc/FixedSizedArena.h"
#include "../../../Misc/Common.h"
#include "Inst.h"
#include "Syntax.h"

AGZ_NS_BEG(AGZ::VMEngExImpl)

template<typename CP>
class Program
{
    Inst<CP> *insts_;
    uint32_t instCapacity_;
    uint32_t instCount_;

    uint8_t relativeOffsetCount_;

    void Release() const
    {
        if(insts_)
            delete[] insts_;
    }

public:

    using Self = Program<CP>;

    Program()
        : insts_(nullptr), instCapacity_(0), instCount_(0),
        relativeOffsetCount_(0)
    {

    }

    explicit Program(uint32_t capacity)
        : instCapacity_(capacity), instCount_(0),
        relativeOffsetCount_(0)
    {
        AGZ_ASSERT(capacity > 0);
        insts_ = new Inst<CP>[capacity];
    }

    Program(Self &&moveFrom) noexcept
        : insts_(moveFrom.insts_),
        instCapacity_(moveFrom.instCapacity_),
        instCount_(moveFrom.instCount_),
        relativeOffsetCount_(moveFrom.relativeOffsetCount_)
    {
        moveFrom.insts_ = nullptr;
        moveFrom.instCapacity_ = 0;
        moveFrom.instCount_ = 0;

        moveFrom.relativeOffsetCount_ = 0;
    }

    Self &operator=(Self &&moveFrom) noexcept
    {
        Release();

        insts_ = moveFrom.insts_;
        instCapacity_ = moveFrom.instCapacity_;
        instCount_ = moveFrom.instCount_;

        relativeOffsetCount_ = moveFrom.relativeOffsetCount_;

        moveFrom.insts_ = nullptr;
        moveFrom.instCapacity_ = 0;
        moveFrom.instCount_ = 0;

        moveFrom.relativeOffsetCount_ = 0;

        return *this;
    }

    ~Program()
    {
        Release();
    }

    Program(const Self &) = delete;
    Self &operator=(const Self &) = delete;

    bool Available() const
    {
        return insts_ != nullptr;
    }

    uint32_t Capacity() const
    {
        return instCapacity_;
    }

    uint32_t Size() const
    {
        return instCount_ + (relativeOffsetCount_ ? 1 : 0);
    }

    Inst<CP> *Emit(const Inst<CP> &inst)
    {
        AGZ_ASSERT(Available() && Size() < Capacity());
        if(relativeOffsetCount_)
        {
            auto ret = &insts_[instCount_ + 1];
            *ret = inst;
            instCount_ += 2;
            relativeOffsetCount_ = 0;
            return ret;
        }

        auto ret = &insts_[instCount_];
        *ret = inst;
        ++instCount_;
        return ret;
    }

    uint32_t GetInstIndex(const Inst<CP> *inst) const
    {
        AGZ_ASSERT(Available() && inst);
        AGZ_ASSERT(inst - insts_ < instCount_);
        return inst - insts_;
    }

    uint32_t *EmitRelativeOffset(uint32_t value = 0)
    {
        AGZ_ASSERT(Available());
        auto ret = &insts_[instCount_].instArrUnit[relativeOffsetCount_];
        *ret = value;
        if(++relativeOffsetCount_ == 3)
        {
            relativeOffsetCount_ = 0;
            instCount_ += 1;
        }
        AGZ_ASSERT(Size() <= Capacity());
        return ret;
    }

    const Inst<CP> &GetInst(size_t idx) const
    {
        AGZ_ASSERT(idx < instCount_);
        return insts_[idx];
    }

    const uint32_t *GetRelativeOffsetArray(size_t instIdx) const
    {
        AGZ_ASSERT(instIdx + 1 < instCount_);
        return &insts_[instIdx + 1].instArrUnit[0];
    }
};

template<typename CS>
class Backend
{
    using I = Inst<typename CS::CodePoint>;
    using Node = ASTNode<typename CS::CodePoint>;

    struct BPUnit
    {
        uint32_t srcPos;
        int32_t *relOffset;
    };

    using BP = std::list<BPUnit>;

public:

    using CP = typename CS::CodePoint;

    Program<CP> Generate(const ASTNode<CP> *ast, size_t *saveSlotCount)
    {
        AGZ_ASSERT(ast && saveSlotCount);
        AGZ_ASSERT(!prog_ && !saveSlotCount_);
        AGZ_ASSERT(!inCharExpr_ && canSave_);

        Program<CP> prog(CountInst(ast) + 1);
        prog_ = &prog;

        auto bps = GenerateImpl(ast);
        auto match = prog_->Emit(NewInst(InstType::Match));
        FillBP(bps, prog_->GetInstIndex(match));

        *saveSlotCount = saveSlotCount_;
        return std::move(prog);
    }

private:

    Program<CP> *prog_ = nullptr;
    size_t saveSlotCount_ = 0;
    bool inCharExpr_ = false;
    bool canSave_ = true;

    static I NewInst(InstType type)
    {
        I ret;
        ret.type = type;
        return ret;
    }

    static void FillBP(BP &bps, uint32_t dstPos)
    {
        for(auto &unit : bps)
        {
            *unit.relOffset = static_cast<int32_t>(dstPos)
                - static_cast<int32_t>(unit.srcPos);
        }
        bps.clear();
    }

    static uint32_t CountInst(const Node *node)
    {
        AGZ_ASSERT(node);
        // TODO
        return 0;
    }

    void GenerateImpl(const Node *node)
    {
        AGZ_ASSERT(node);
        // TODO
    }
};

AGZ_NS_END(AGZ::VMEngExImpl)
