#pragma once

#include <limits>
#include <optional>
#include <vector>

#include "../../Alloc/FixedSizedArena.h"
#include "../../Misc/Common.h"

AGZ_NS_BEG(AGZ::VMEngine)

template<typename CP>
struct Inst
{
    enum Type
    {
        Begin, End,
        Dot, Char,
        Save,
        Alter, Jump, Branch,
        Match
    } op;

    union
    {
        CP codePoint;
        Inst<CP> *jumpDest;
        Inst<CP> *branchDest[2];
        struct
        {
            size_t alterCount;
            Inst<CP> *alterDest;
        };
        size_t saveSlot;
    };
    
    size_t lastStep;
};

struct ASTNode;

struct AlterListNode
{
    ASTNode *dest;
    AlterListNode *next;
};

struct ASTNode
{
    enum Type
    {
        Begin, End,
        Dot, Char,
        Save,
        Cat, Alter,
        Star, Plus, Ques
    } type;

    union
    {
        char32_t codePoint;
        ASTNode *catDest[2];
        ASTNode *orDest[2];
        struct
        {
            size_t alterCount;
            AlterListNode *alterDest;
        };
        ASTNode *starDest;
        ASTNode *plusDest;
        ASTNode *quesDest;
    };
};

template<typename CS>
class Parser
{
public:

    ASTNode *Parse(const StringView<CS> &regexp)
    {
        astNodeArena_.Release();
        alterNodeArena_.Release();

        auto cpRange  = regexp.CodePoints();
        cur_          = cpRange.begin();
        end_          = cpRange.end();
        nextSaveSlot_ = 0;

        auto ret = ParseRegex();
        if(cur_ != end_)
            Error();
        return ret;
    }

private:

    using CP      = typename CS::CodePoint;
    using It      = typename CS::Iterator;

    SmallObjectArena<ASTNode>       astNodeArena_;
    SmallObjectArena<AlterListNode> alterNodeArena_;

    It cur_, end_;
    size_t nextSaveSlot_;

    void Error() { throw ArgumentException("Invalid regular expression"); }

    bool End() const        { return cur_ == end_; }
    CP   Char() const       { AGZ_ASSERT(!End()); return *cur_; }
    CP   CurAndAdv()        { CP ret = Char(); Advance(); return ret; }
    void ErrIfEnd() const   { if(End()) Error(); }
    void Advance()          { AGZ_ASSERT(!End()); ++cur_; }
    bool Match(CP c) const  { return !End() && Char() == c; }
    bool AdvanceIf(CP c)    { return Match(c) ? Advance(), true : false; }
    void AdvanceOrErr(CP c) { if(!AdvanceIf(c)) Error(); }
    
    ASTNode *NewASTNode(ASTNode::Type type)
    {
        auto ret = astNodeArena_.Malloc();
        ret->type = type;
        return ret;
    }

    ASTNode *ParseChar()
    {
        if(IsEnd())
            return nullptr;
        CP cp = Char();

        if(cp == '^')
            return NewASTNode(ASTNode::Begin);
        if(cp == '$')
            return NewASTNode(ASTNode::End);
        if(cp == '&')
        {
            if(savable_)
                return NewASTNode(ASTNode::Save);
            Error();
        }

        switch(cp)
        {
        case '[':
        case ']':
        case '(':
        case ')':
        case '+':
        case '*':
        case '?':
        case '&':
            return nullptr;
        default:
            break;
        }

        Advance();
        if(cp == '\\')
        {
            ErrIfEnd();
            CP next = CurAndAdv();
            switch(cp)
            {
            case 'b': cp = '\b'; break;
            case 'n': cp = '\n'; break;
            case 't': cp = '\t'; break;
            case 'r': cp = '\r'; break;
            case '[':
            case ']':
            case '(':
            case ')':
            case '+':
            case '*':
            case '?':
            case '^':
            case '$':
            case '&':
            case '\\':
                cp = next; break;
            default:
                Error();
            }
        }

        ASTNode *ret = astNodeArena_.Malloc();
        if(cp == '.')
            ret->type = ASTNode::Dot;
        else
        {
            ret->type = ASTNode::Char;
            ret->codePoint = cp;
        }
        return ret;
    }

    ASTNode *ParseCore()
    {
        if(AdvanceIf('('))
        {
            auto ret = ParseRegex();
            AdvanceOrErr(')');
            return ret;
        }

        if(AdvanceIf('['))
        {
            ASTNode *alterNode = NewASTNode(ASTNode::Alter);
            alterNode->alterCount = 0;
            alterNode->alterDest  = nullptr;

            ASTNode *sub;
            while((sub = ParseFac()))
            {
                AlterListNode *listNode = alterNodeArena_.Malloc();
                listNode->dest = sub;
                listNode->next = alterNode->alterDest;
                alterNode->alterDest = listNode;
                ++alterNode->alterCount;
            }

            if(!alterNode->alterCount)
                Error();

            AdvanceOrErr(']');

            return alterNode;
        }

        return ParseChar();
    }

    ASTNode *ParseFac()
    {
        auto last = ParseCore();
        if(!last)
            return nullptr;

        while(true)
        {
            if(AdvanceIf('*'))
            {
                ASTNode *newNode = NewASTNode(ASTNode::Star);
                newNode->starDest = last;
                last = newNode;
            }
            else if(AdvanceIf('+'))
            {
                ASTNode *newNode = NewASTNode(ASTNode::Plus);
                newNode->plusDest = last;
                last = newNode;
            }
            else if(AdvanceIf('?'))
            {
                ASTNode *newNode = NewASTNode(ASTNode::Ques);
                newNode->quesDest = last;
                last = newNode;
            }
            else
                return last;
        }
    }

    ASTNode *ParseRegex()
    {
        ASTNode *last = ParseFac();
        if(!last)
            return nullptr;
        ASTNode *right;
        while((right = ParseFac()))
        {
            ASTNode *newNode = NewASTNode(ASTNode::Cat);
            newNode->catDest[0] = last;
            newNode->catDest[1] = right;
            last = newNode;
        }
        return last;
    }
};

template<typename CP>
class Program
{
public:

    using Self = Program<CP>;
    
    Program()
        : nextInst_(0), instCount_(0), insts_(nullptr)
    {
        
    }

    Program(size_t instCount)
        : nextInst_(0), instCount_(instCount)
    {
        AGZ_ASSERT(instCount);
        insts_ = new Inst<CP>[instCount];
    }

    Program(Self &&moveFrom)
        : insts_(moveFrom.insts_)
    {
        moveFrom.insts_ = nullptr;
    }

    Self &operator=(Self &&moveFrom)
    {
        if(insts_)
            delete insts_;
        insts_ = moveFrom.insts_;
        moveFrom.insts_ = nullptr;
        return *this;
    }

    Program(const Self &)         = delete;
    Self &operator=(const Self &) = delete;

    ~Program()
    {
        if(insts_)
            delete insts_;
    }

    bool IsAvailable() const
    {
        return insts_ != nullptr;
    }
    
    Inst &operator[](size_t idx)
    {
        AGZ_ASSERT(insts_ && idx < nextInst_);
        return insts_[idx];
    }

    const Inst &operator[](size_t idx) const
    {
        AGZ_ASSERT(insts_ && idx < nextInst_);
        return insts_[idx];
    }

    Insts *Emit(const Inst<CP> &inst)
    {
        AGZ_ASSERT(insts_ && nextInst_ < instCount_);
        insts_[nextInst_] = inst;
        return &insts[nextInst_++];
    }

    Inst<CP> *GetNextPtr() const { return &insts_[nextInst_]; }

private:

    size_t nextInst_;
    size_t instCount_;
    Inst<CP> *insts_;
};

template<typename CS>
class Compiler
{
public:

    using CP = typename CS::CodePoint;
    using BP = std::list<Inst**>;

    Program<CP> Compile(const StringView<CS> &regex,
                        size_t *saveSlotCount)
    {
        AGZ_ASSERT(prog && saveSlotCount);

        auto ast = Parser<CP>::Parse(regex);

        Program ret(CountInst(ast));
        prog_ = &ret;
        saveSlotCount_ = 0;

        auto bps = Generate(ast);
        FillBP(bps, prog_->Emit(MakeInst(Inst<CP>::Match)));

        *saveSlotCount = saveSlotCount_;
        return std::move(ret);
    }

private:

    Program<CP> *prog_;
    size_t saveSlotCount_;

    using I = Inst<CP>;
    I MakeInst(I::Type type) { return I { type }; }

    void FillBP(BP &bps, Inst<CP> *val)
    {
        for(auto p : bps)
            *p = val;
        bps.clear();
    }

    size_t CountInst(ASTNode *n)
    {
        AGZ_ASSERT(n);
        switch(n->type)
        {
        case ASTNode::Begin:
        case ASTNode::End:
        case ASTNode::Dot:
        case ASTNode::Char:
        case ASTNode::Save:
            return 1;
        case ASTNode::Cat:
            return CountInst(n->catDest[0]) + CountInst(n->catDest[1]);
        case ASTNode::Alter:
            {
                size_t ret = 0;
                auto nl = n->alterDest;
                while(nl)
                {
                    ret += CountInst(nl->dest);
                    nl = nl->next;
                }
                return ret;
            }
        case ASTNode::Star:
            return 2 + CountInst(n->starDest);
        case ASTNode::Plus:
            return 1 + CountInst(n->plusDest);
        case ASTNode::Ques:
            return 1 + CountInst(n->quesDest);
        default:
            Unreachable();
        }
        Unreachable();
    }

    BP Generate(ASTNode *node)
    {
        AGZ_ASSERT(node);

        switch(node->type)
        {
        case ASTNode::Begin:
            prog_->Emit(MakeInst(I::Begin));
            return { };
        case ASTNode::End:
            prog_->Emit(MakeInst(I::End));
            return { };
        case ASTNode::Dot:
            prog_->Emit(MakeInst(I::Dot));
            return { };
        case ASTNode::Char:
            prog_->Emit(MakeInst(I::Char))->codePoint = node->codePoint;
            return { };
        case ASTNode::Save:
            prog_->Emit(MakeInst(I::Save))->saveSlot = saveSlotCount_++;
            return { };
        case ASTNode::Cat:
            FillBP(Generate(node->catDest[0]));
            return Generate(node->catDest[1]);
        case ASTNode::Alter:
            return GenerateAlter(node);
        case ASTNode::Star:
            return GenerateStar(node);
        case ASTNode::Plus:
            return GeneratePlus(node);
        case ASTNode::Ques:
            return GenerateQues(node);
        default;
            Unreachable();
        }
        Unreachable();
    }

    BP GenerateAlter(ASTNode *node)
    {
        AGZ_ASSERT(node && node->type == ASTNode::Alter);

        size_t alterCount = node->alterCount;
        Inst<CP> *alterDest = new Inst<CP>[alterCount];

        prog_->Emit(MakeInst(I::Alter));
        auto alterDestNode = node->alterDest;
        AGZ_ASSERT(alterDestNode);

        alterDest[0] = prog_->GetNextPtr();
        BP ret(Generate(alterDestNode->dest));

        for(size_t i = 1; i < alterCount; ++i)
        {
            alterDestNode = alterDestNode->next;
            AGZ_ASSERT(alterDestNode);

            ret.push_back(&Emit(MakeInst(I::Jump))->jumpDest);
            alterDest[i] = prog_->GetNextPtr();
            ret.splice(ret.end(), Generate(alterDestNode->dest));
        }

        return std::move(ret);
    }

    BP GenerateStar(ASTNode *node)
    {
        AGZ_ASSERT(node && node->type == ASTNode::Star);

        auto branch = prog_->Emit(MakeInst(I::Branch));
        branch->branchDest[0] = prog_->GetNextPtr();

        auto bps = Generate(node->starDest);
        FillBP(bps, branch);

        prog_->Emit(MakeInst(I::Jump))->jumpDest = branch;

        return { &branch->branchDest[1] };
    }

    BP GeneratePlus(ASTNode *node)
    {
        AGZ_ASSERT(node && node->type == ASTNode::Plus);

        auto p = prog_->GetNextPtr();
        auto bps = Generate(node->plusDest);

        auto branch = prog_->Emit(MakeInst(I::Branch));
        branch->branchDest[0] = p;

        FillBP(bps, branch);

        return { &branch->branchDest[1] };
    }

    BP GenerateQues(ASTNode *node)
    {
        AGZ_ASSERT(node && node->type == ASTNode::Ques);

        auto branch = prog_->Emit(MakeInst(I::Branch));
        branch->branchDest[0] = prog_->GetNextPtr();
        auto ret = Generate(node->quesDest);
        ret.push_back(&branch->branchDest[1]);
        return std::move(ret);
    }
};

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
struct Thread
{
    Thread(Inst<CP> *pc, SaveSlots &&saveSlots)
        : pc(pc), saveSlots(std::move(saveSlots))
    {

    }
    
    Inst<CP> *pc;
    SaveSlots saveSlots;
};

template<typename CS>
class Machine
{
public:
    using CU = typename CS::CodeUnit;
    using CP = typename CS::CodePoint;
    
    using Arena = FixedSizedArena<>;
    using Interval = std::pair<size_t, size_t>;

private:

    mutable Program<CP> prog_;
    mutable size_t slotCount_;
    mutable String<CS> regex_;
    
    Thread<CP> *NewThread(Arena &arena, Inst<CP> *pc,
                          SaveSlots &&saveSlots)
    {
        Thread<CP> *ret = arena.Malloc<Thread<CP>>();
        return new(ret) Thread<CP>(pc, std::move(saveSlots));
    }
    
    // TODO
};

AGZ_NS_END(AGZ::VMEngine)
