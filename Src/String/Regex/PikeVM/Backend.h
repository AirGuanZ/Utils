#pragma once

#include <type_traits>

#include "../../../Alloc/ObjArena.h"
#include "../../../Misc/Common.h"
#include "../../../Misc/Exception.h"
#include "Inst.h"
#include "Syntax.h"

namespace AGZ::PikeVM {

/*
    A|B =>    Branch(L0, L1)
           L0 Inst(A) -> Out
              Jump(Out)
           L1 Inst(B) -> Out

    A B C =>    Inst(A) -> L0
             L0 Inst(B) -> L1
             L1 Inst(C) -> Out

    A* => L0 Branch(L1, Out)
          L1 Inst(A) -> L0
             Jump(L0)

    A+ => L0 Inst(A) -> L1
          L1 Branch(L0, Out)

    A? =>    Branch(L0, Out)
          L0 Inst(A) -> Out

    A{m} =>         Inst(A) -> L0
            L0      Inst(A) -> L1
            L1      Inst(A) -> L2
                    ...
            L_{m-2} Inst(A) -> Out

    A{m, n} =>         Inst(A{m}) -> L0
               L0      Alter(L1, L2, ..., L_{n - m}, Out)
               L1      Inst(A) -> L2
               L2      Inst(A) -> L3
                       ...
               L_{n-m} Inst(A) -> Out

    (A) => Inst(A) -> Out

    [ABC] => if in expr then
                 Inst(A|B|C)
             else
                 Inst(@{A|B|C})

    A|B|C|D => Inst(A)
               if_true_set_true_and_jump(Out)
               Inst(B)
               if_true_set_true_and_jump(Out)
               Inst(C)
               if_true_set_true_and_jump(Out)
               Inst(D)
               if_true_set_true_and_jump(Out)
               set_false

    A&B&C&D => Inst(A)
               if_false_set_false_and_jump(Out)
               ...
               Inst(D)
               if_false_set_false_and_jump(Out)
               set_true

    !A => Inst(A)
          bool_not
*/

constexpr size_t INST_REL_OFFSET_CAPACITY = sizeof(Inst<char32_t>) / sizeof(int32_t);

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

    Program(const Self &)         = delete;
    Self &operator=(const Self &) = delete;

    bool Available() const
    {
        return insts_ != nullptr;
    }

    bool Full() const
    {
        return Size() == Capacity();
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
        AGZ_ASSERT(inst >= &insts_[0]);
        AGZ_ASSERT(static_cast<uint32_t>(inst - &insts_[0]) < instCount_);
        return static_cast<uint32_t>(inst - insts_);
    }

    int32_t *EmitRelativeOffset(int32_t value = 0)
    {
        AGZ_ASSERT(Available());
        auto ret = &insts_[instCount_].instArrUnit[relativeOffsetCount_];
        *ret = value;
        if(++relativeOffsetCount_ == INST_REL_OFFSET_CAPACITY)
        {
            relativeOffsetCount_ = 0;
            instCount_ += 1;
        }
        AGZ_ASSERT(Size() <= Capacity());
        return ret;
    }

    Inst<CP> &GetInst(size_t idx)
    {
        AGZ_ASSERT(idx < instCount_);
        return insts_[idx];
    }

    const int32_t *GetRelativeOffsetArray(size_t instIdx) const
    {
        AGZ_ASSERT(instIdx + 1 < Size());
        return &insts_[instIdx + 1].instArrUnit[0];
    }

    int32_t *GetRelativeOffsetArray(size_t instIdx)
    {
        AGZ_ASSERT(instIdx + 1 < Size());
        return &insts_[instIdx + 1].instArrUnit[0];
    }

    uint32_t GetNextInstIndex() const
    {
        AGZ_ASSERT(Available() && Size() < Capacity());
        return Size();
    }

    void ReinitLastSteps()
    {
        AGZ_ASSERT(Full());
        for(uint32_t i = 0; i < instCount_; ++i)
        {
            if(insts_[i].type != InstType::Alter)
                insts_[i].lastStep = std::numeric_limits<uint32_t>::max();
            else
            {
                insts_[i].lastStep = std::numeric_limits<uint32_t>::max();
                i += (insts_[i].dataAlter.count / INST_REL_OFFSET_CAPACITY)
                   + (insts_[i].dataAlter.count / INST_REL_OFFSET_CAPACITY != 0);
            }
        }
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

        AGZ_ASSERT(prog_->Full());

        *saveSlotCount = saveSlotCount_;
        return std::move(prog);
    }

private:

    Program<CP> *prog_ = nullptr;
    size_t saveSlotCount_ = 0;
    bool inCharExpr_ = false;
    bool canSave_ = true;

    [[noreturn]] static void Error()
    {
        throw ArgumentException("Ill-formed regular expression");
    }

    bool DisableSaving()
    {
        bool ret = canSave_;
        canSave_ = false;
        return ret;
    }

    void SetSaving(bool canSave)
    {
        canSave_ = canSave;
    }

    static I NewInst(InstType type)
    {
        I ret;
        ret.type = type;
        return ret;
    }

    static void FillBP(BP &bps, uint32_t dstPos)
    {
        for(auto &unit : bps)
            *unit.relOffset = ComputeOffset(unit.srcPos, dstPos);
        bps.clear();
    }

    static int32_t ComputeOffset(uint32_t src, uint32_t dst)
    {
        return static_cast<int32_t>(dst) - static_cast<int32_t>(src);
    }

    static uint32_t AlterSize(uint32_t alterDestCount)
    {
        return 1 + alterDestCount / INST_REL_OFFSET_CAPACITY
                 + (alterDestCount % INST_REL_OFFSET_CAPACITY != 0);
    }

    static uint32_t CountInst(const Node *node, bool inExpr = false)
    {
        AGZ_ASSERT(node);
        switch(node->type)
        {
        case ASTType::Begin:
        case ASTType::End:
        case ASTType::Save:
        case ASTType::CharSingle:
        case ASTType::CharAny:
        case ASTType::CharDecDigit:
        case ASTType::CharHexDigit:
        case ASTType::CharAlpha:
        case ASTType::CharWordChar:
        case ASTType::CharWhitespace:
            return 1;
        case ASTType::Cat:
            return CountInst(node->dataCat.dest[0], inExpr)
                 + CountInst(node->dataCat.dest[1], inExpr);
        case ASTType::Or:
            return CountInst(node->dataOr.dest[0], inExpr)
                 + CountInst(node->dataOr.dest[1], inExpr)
                 + 2;
        case ASTType::Star:
            return CountInst(node->dataStar.dest, inExpr) + 2;
        case ASTType::Plus:
            return CountInst(node->dataPlus.dest, inExpr) + 1;
        case ASTType::Ques:
            return CountInst(node->dataQues.dest, inExpr) + 1;
        case ASTType::Repeat:
            if(node->dataRepeat.lst > node->dataRepeat.fst)
                return node->dataRepeat.lst
                     * CountInst(node->dataRepeat.dest, inExpr)
                     + AlterSize(node->dataRepeat.lst
                               - node->dataRepeat.fst + 1);
            return node->dataRepeat.fst
                 * CountInst(node->dataRepeat.dest, inExpr);
        case ASTType::CharClass:
            if(inExpr)
                return node->dataCharClass.memCnt * 2 + 1;
            return node->dataCharClass.memCnt * 2 + 2;
        case ASTType::CharExpr:
            return CountInst(node->dataCharExpr.expr, true) + 1;
        case ASTType::CharExprAnd:
            return CountInst(node->dataCharExprAnd.left, inExpr)
                 + CountInst(node->dataCharExprAnd.right, inExpr)
                 + 3;
        case ASTType::CharExprOr:
            return CountInst(node->dataCharExprOr.left, inExpr)
                 + CountInst(node->dataCharExprOr.right, inExpr)
                 + 3;
        case ASTType::CharExprNot:
            return CountInst(node->dataCharExprNot.dest, inExpr) + 1;
        default:
            Unreachable();
        }
    }

    BP EmitParamlessInst(InstType type)
    {
        prog_->Emit(NewInst(type));
        return { };
    }

    [[nodiscard]] BP GenerateImpl(const Node *node)
    {
        AGZ_ASSERT(node);
        switch(node->type)
        {
        case ASTType::Begin:
            return EmitParamlessInst(InstType::Begin);
        case ASTType::End:
            return EmitParamlessInst(InstType::End);
        case ASTType::Save:
            if(canSave_)
            {
                prog_->Emit(NewInst(InstType::Save))
                     ->dataSave.slot = static_cast<uint32_t>(saveSlotCount_++);
            }
            else
                Error();
            return { };
        case ASTType::Cat:
            return GenerateCatImpl(node);
        case ASTType::Or:
            return GenerateOrImpl(node);
        case ASTType::Star:
            return GenerateStarImpl(node);
        case ASTType::Plus:
            return GeneratePlusImpl(node);
        case ASTType::Ques:
            return GenerateQuesImpl(node);
        case ASTType::Repeat:
            return GenerateRepeatImpl(node);
        case ASTType::CharSingle:
            return GenerateCharSingleImpl(node);
        case ASTType::CharAny:
            return EmitParamlessInst(inCharExpr_ ? InstType::CharExprAny
                                                 : InstType::CharAny);
        case ASTType::CharClass:
            return GenerateCharClassImpl(node);
        case ASTType::CharDecDigit:
            return EmitParamlessInst(inCharExpr_ ? InstType::CharExprDecDigit
                                                 : InstType::CharDecDigit);
        case ASTType::CharHexDigit:
            return EmitParamlessInst(inCharExpr_ ? InstType::CharExprHexDigit
                                                 : InstType::CharHexDigit);
        case ASTType::CharAlpha:
            return EmitParamlessInst(inCharExpr_ ? InstType::CharExprAlpha
                                                 : InstType::CharAlpha);
        case ASTType::CharWordChar:
            return EmitParamlessInst(inCharExpr_ ? InstType::CharExprWordChar
                                                 : InstType::CharWordChar);
        case ASTType::CharWhitespace:
            return EmitParamlessInst(inCharExpr_ ? InstType::CharExprWhitespace
                                                 : InstType::CharWhitespace);
        case ASTType::CharExpr:
            return GenerateCharExprImpl(node);
        case ASTType::CharExprAnd:
            return GenerateCharExprAndImpl(node);
        case ASTType::CharExprOr:
            return GenerateCharExprOrImpl(node);
        case ASTType::CharExprNot:
            return GenerateCharExprNotImpl(node);
        default:
            Unreachable();
        }
    }

    /*
        A B C
               Inst(A) -> L0
            L0 Inst(B) -> L1
            L1 Inst(C) -> Out
    */
    BP GenerateCatImpl(const Node *node)
    {
        AGZ_ASSERT(node && node->type == ASTType::Cat);
        auto bps = GenerateImpl(node->dataCat.dest[0]);
        FillBP(bps, prog_->GetNextInstIndex());
        AGZ_ASSERT(bps.empty());
        return GenerateImpl(node->dataCat.dest[1]);
    }

    /*
        A|B
               Branch(L0, L1)
            L0 Inst(A) -> Out
               Jump(Out)
            L1 Inst(B) -> Out
    */
    BP GenerateOrImpl(const Node *node)
    {
        AGZ_ASSERT(node && node->type == ASTType::Or);
        auto branch = prog_->Emit(NewInst(InstType::Branch));

        branch->dataBranch.dest[0] = ComputeOffset(
            prog_->GetInstIndex(branch), prog_->GetNextInstIndex());
        auto ret = GenerateImpl(node->dataOr.dest[0]);

        auto jump = prog_->Emit(NewInst(InstType::Jump));
        ret.push_back({ prog_->GetInstIndex(jump), &jump->dataJump.offset });

        branch->dataBranch.dest[1] = ComputeOffset(
            prog_->GetInstIndex(branch), prog_->GetNextInstIndex());
        ret.splice(ret.end(), GenerateImpl(node->dataOr.dest[1]));

        return ret;
    }

    /*
        A*
            L0 Branch(L1, Out)
            L1 Inst(A) -> L0
               Jump(L0)
    */
    BP GenerateStarImpl(const Node *node)
    {
        AGZ_ASSERT(node && node->type == ASTType::Star);

        auto branch = prog_->Emit(NewInst(InstType::Branch));
        branch->dataBranch.dest[0] = ComputeOffset(
            prog_->GetInstIndex(branch), prog_->GetNextInstIndex());

        auto oldCanSave = DisableSaving();
        auto bps = GenerateImpl(node->dataStar.dest);
        SetSaving(oldCanSave);
        FillBP(bps, prog_->GetInstIndex(branch));

        auto jump = prog_->Emit(NewInst(InstType::Jump));
        jump->dataJump.offset = ComputeOffset(
            prog_->GetInstIndex(jump), prog_->GetInstIndex(branch));

        AGZ_ASSERT(bps.empty());
        return { { prog_->GetInstIndex(branch),
                   &branch->dataBranch.dest[1] } };
    }

    /*
        A+
            L0 Inst(A) -> L1
            L1 Branch(L0, Out)
    */
    BP GeneratePlusImpl(const Node *node)
    {
        AGZ_ASSERT(node && node->type == ASTType::Plus);

        auto begin = prog_->GetNextInstIndex();

        auto oldCanSave = DisableSaving();
        auto bps = GenerateImpl(node->dataPlus.dest);
        SetSaving(oldCanSave);

        FillBP(bps, prog_->GetNextInstIndex());

        auto branch = prog_->Emit(NewInst(InstType::Branch));
        branch->dataBranch.dest[0] = ComputeOffset(
            prog_->GetInstIndex(branch), begin);

        AGZ_ASSERT(bps.empty());
        return { { prog_->GetInstIndex(branch),
                   &branch->dataBranch.dest[1] } };
    }

    /*
        A?
               Branch(L0, Out)
            L0 Inst(A) -> Out
    */
    BP GenerateQuesImpl(const Node *node)
    {
        auto branch = prog_->Emit(NewInst(InstType::Branch));
        branch->dataBranch.dest[0] = ComputeOffset(
            prog_->GetInstIndex(branch), prog_->GetNextInstIndex());

        auto oldCanSave = DisableSaving();
        auto ret = GenerateImpl(node->dataQues.dest);
        SetSaving(oldCanSave);

        ret.push_back({ prog_->GetInstIndex(branch),
                        &branch->dataBranch.dest[1] });
        return ret;
    }

    /*
        A{m}
                    Inst(A) -> L0
            L0      Inst(A) -> L1
            L1      Inst(A) -> L2
                    ...
            L_{m-2} Inst(A) -> Out

        A{m, n}
                    Inst(A{m}) -> L0
            L0      Alter(L1, L2, ..., L_{n - m}, Out)
            L1      Inst(A) -> L2
            L2      Inst(A) -> L3
                    ...
            L_{n-m} Inst(A) -> Out
    */
    BP GenerateRepeatImpl(const Node *node)
    {
        BP bps;
        auto oldCanSave = DisableSaving();

        for(uint32_t i = 0; i < node->dataRepeat.fst; ++i)
        {
            FillBP(bps, prog_->GetNextInstIndex());
            bps = GenerateImpl(node->dataRepeat.dest);
        }

        if(node->dataRepeat.lst > node->dataRepeat.fst)
        {
            uint32_t num = node->dataRepeat.lst - node->dataRepeat.fst;
            AGZ_ASSERT(num > 0);

            FillBP(bps, prog_->GetNextInstIndex());

            auto alter = prog_->Emit(NewInst(InstType::Alter));
            alter->dataAlter.count = num + 1;
            auto alterIdx = prog_->GetInstIndex(alter);
            for(uint32_t i = 0; i < num + 1; ++i)
                prog_->EmitRelativeOffset();
            auto alterDests = prog_->GetRelativeOffsetArray(
                prog_->GetInstIndex(alter));

            AGZ_ASSERT(bps.empty());
            for(uint32_t i = 0; i < num; ++i)
            {
                alterDests[i] = ComputeOffset(
                    alterIdx, prog_->GetNextInstIndex());
                FillBP(bps, prog_->GetNextInstIndex());
                bps = GenerateImpl(node->dataRepeat.dest);
            }

            bps.push_back({ alterIdx, &alterDests[num] });
        }

        SetSaving(oldCanSave);
        return bps;
    }

    BP GenerateCharSingleImpl(const Node *node)
    {
        AGZ_ASSERT(node && node->type == ASTType::CharSingle);

        if(inCharExpr_)
        {
            prog_->Emit(NewInst(InstType::CharExprSingle))
                ->dataCharExprSingle.codePoint
                    = node->dataCharSingle.codePoint;
        }
        else
        {
            prog_->Emit(NewInst(InstType::CharSingle))
                ->dataCharSingle.codePoint
                    = node->dataCharSingle.codePoint;
        }

        return { };
    }

    /*
        [ABC]
            if in expr then
                Inst(A|B|C)
            else
                Inst(@{A|B|C})

        A|B|C|D
            Inst(A)
            if_true_set_true_and_jump(Out)
            Inst(B)
            if_true_set_true_and_jump(Out)
            Inst(C)
            if_true_set_true_and_jump(Out)
            Inst(D)
            if_true_set_true_and_jump(Out)
            set_false
    */
    BP GenerateCharClassImpl(const Node *node)
    {
        AGZ_ASSERT(node && node->type == ASTType::CharClass);

        if(!node->dataCharClass.memCnt)
            Error();

        BP bps;
        for(auto mem = node->dataCharClass.mems;
            mem; mem = mem->next)
        {
            if(mem->isRange)
            {
                auto range = prog_->Emit(NewInst(InstType::CharExprRange));
                range->dataCharExprRange.fst = mem->fst;
                range->dataCharExprRange.lst = mem->snd;
            }
            else
            {
                auto single = prog_->Emit(NewInst(InstType::CharExprSingle));
                single->dataCharExprSingle.codePoint = mem->fst;
            }

            auto ITSTAJ = prog_->Emit(NewInst(InstType::CharExprITSTAJ));
            bps.push_back({ prog_->GetInstIndex(ITSTAJ),
                            &ITSTAJ->dataITSTAJ.offset });
        }

        prog_->Emit(NewInst(InstType::CharExprSetFalse));

        if(!inCharExpr_)
        {
            FillBP(bps, prog_->GetNextInstIndex());
            prog_->Emit(NewInst(InstType::CharExprEnd));
        }

        return bps;
    }

    BP GenerateCharExprImpl(const Node *node)
    {
        AGZ_ASSERT(node && node->type == ASTType::CharExpr);

        if(inCharExpr_)
            Error();
        inCharExpr_ = true;

        auto bps = GenerateImpl(node->dataCharExpr.expr);
        auto endExpr = prog_->Emit(NewInst(InstType::CharExprEnd));
        FillBP(bps, prog_->GetInstIndex(endExpr));

        AGZ_ASSERT(bps.empty());
        inCharExpr_ = false;
        return { };
    }

    /*
        A&B
            Inst(A)
            if_false_set_false_and_jump(Out)
            Inst(B)
            if_false_set_false_and_jump(Out)
            set_true
    */
    BP GenerateCharExprAndImpl(const Node *node)
    {
        BP ret;

        auto bps = GenerateImpl(node->dataCharExprAnd.left);
        FillBP(bps, prog_->GetNextInstIndex());
        auto IFSFAJ = prog_->Emit(NewInst(InstType::CharExprIFSFAJ));
        ret.push_back({ prog_->GetInstIndex(IFSFAJ),
                        &IFSFAJ->dataIFSFAJ.offset });

        bps = GenerateImpl(node->dataCharExprAnd.right);
        FillBP(bps, prog_->GetNextInstIndex());
        IFSFAJ = prog_->Emit(NewInst(InstType::CharExprIFSFAJ));
        ret.push_back({ prog_->GetInstIndex(IFSFAJ),
                        &IFSFAJ->dataIFSFAJ.offset });

        prog_->Emit(NewInst(InstType::CharExprSetTrue));

        AGZ_ASSERT(bps.empty());
        return ret;
    }

    /*
        A|B
            Inst(A)
            if_true_set_true_and_jump(Out)
            Inst(B)
            if_true_set_true_and_jump(Out)
            set_false
    */
    BP GenerateCharExprOrImpl(const Node *node)
    {
        BP ret;

        auto bps = GenerateImpl(node->dataCharExprOr.left);
        FillBP(bps, prog_->GetNextInstIndex());
        auto ITSTAJ = prog_->Emit(NewInst(InstType::CharExprITSTAJ));
        ret.push_back({ prog_->GetInstIndex(ITSTAJ),
                        &ITSTAJ->dataIFSFAJ.offset });

        bps = GenerateImpl(node->dataCharExprOr.right);
        FillBP(bps, prog_->GetNextInstIndex());
        ITSTAJ = prog_->Emit(NewInst(InstType::CharExprITSTAJ));
        ret.push_back({ prog_->GetInstIndex(ITSTAJ),
                        &ITSTAJ->dataIFSFAJ.offset });

        prog_->Emit(NewInst(InstType::CharExprSetFalse));

        AGZ_ASSERT(bps.empty());
        return ret;
    }

    /*
        !A
            Inst(A)
            bool_not
    */
    BP GenerateCharExprNotImpl(const Node *node)
    {
        AGZ_ASSERT(node && node->type == ASTType::CharExprNot);

        auto bps = GenerateImpl(node->dataCharExprNot.dest);
        FillBP(bps, prog_->GetNextInstIndex());

        AGZ_ASSERT(bps.empty());
        return EmitParamlessInst(InstType::CharExprNot);
    }
};

} // namespace AGZ::PikeVM
