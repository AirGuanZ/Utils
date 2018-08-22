#pragma once

#include <limits>
#include <optional>
#include <vector>

#include "../../Alloc/FixedSizedArena.h"
#include "../../Misc/Common.h"
#include "../String/StrAlgo.h"
#include "../String/String.h"

// Regular expression matching with virtual machine
// See https://swtch.com/~rsc/regexp/regexp2.html

/*
    ab      concatenation
    a|b     alternative
    [abc]   alternatives
    a+      one or more
    a*      zero or more
    a?      zero or one
    ^       beginning
    $       end
    &       save point
    .       any character
    a{m}    m times (m > 0)
    a{m, n} m to n times (0 <= m, m <= n, 0 < n)
    <az>    character in [a, z]
    
    <d>     digit 0-9
    <c>     a-z and A-Z
    <w>     alnum and _
    <s>     whitespace
    <h>     hexdigit 0-9 a-f A-F

Grammar:

    Regex := Cat \| Cat \| ... \| Cat
    Cat   := Fac Fac ... Fac
    Fac   := Fac* | Fac+ | Fac? |
             Fac{m} | Fac{m, n} |
             Core
    Core  := (Regex) | [Fac Fac ... Fac] |
             Character | . | & | ^ | $ |
             <Character Character>
*/

AGZ_NS_BEG(AGZ::VMEngineImpl)

using namespace AGZ::StrImpl::StrAlgo;

using std::list;
using std::move;
using std::nullopt;
using std::numeric_limits;
using std::optional;
using std::pair;
using std::vector;

template<typename CP>
struct Inst
{
    enum Type
    {
        Begin, End,
        Dot, Char, CharRange,
        
        // Special character classes
        Digit, Alpha, WordChar, Whitespace, HexDigit,
        
        Save,
        Alter, Jump, Branch,
        Match
    } op;

    union
    {
        CP codePoint;
        char32_t charRange[2];
        Inst<CP> *jumpDest;
        Inst<CP> *branchDest[2];
        struct
        {
            size_t alterCount;
            Inst<CP> **alterDest;
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
        Dot, Char, CharRange,
        Digit, Alpha, WordChar, Whitespace, HexDigit,
        Save,
        Cat, Alter, Or,
        Star, Plus, Ques,
        Repeat, RepeatRange,
    } type;

    union
    {
        char32_t codePoint;
        char32_t charRange[2];
        struct
        {
            size_t repeatCount;
            ASTNode *repeatContent;
        };
        struct
        {
            size_t min;
            size_t max;
            ASTNode *content;
        } repeatRange;
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
        astNodeArena_.ReleaseAll();
        alterNodeArena_.ReleaseAll();

        auto cpRange  = regexp.CodePoints();
        cur_          = cpRange.begin();
        end_          = cpRange.end();

        auto ret = ParseRegex();
        if(cur_ != end_)
            Error();
        return ret;
    }

private:

    using CP = typename CS::CodePoint;
    using It = typename CS::Iterator;

    SmallObjArena<ASTNode>       astNodeArena_;
    SmallObjArena<AlterListNode> alterNodeArena_;

    It cur_, end_;

    bool End() const        { return cur_ == end_;                       }
    CP   Char() const       { AGZ_ASSERT(!End()); return *cur_;          }
    CP   CurAndAdv()        { CP ret = Char(); Advance(); return ret;    }
    void ErrIfEnd() const   { if(End()) Error();                         }
    void Advance()          { AGZ_ASSERT(!End()); ++cur_;                }
    bool Match(CP c) const  { return !End() && Char() == c;              }
    bool AdvanceIf(CP c)    { return Match(c) ? Advance(), true : false; }
    void AdvanceOrErr(CP c) { if(!AdvanceIf(c)) Error();                 }

    [[noreturn]] static AGZ_FORCEINLINE void Error()
    {
        throw ArgumentException("Invalid regular expression");
    }

    ASTNode *NewASTNode(ASTNode::Type type)
    {
        auto ret = astNodeArena_.Malloc();
        ret->type = type;
        return ret;
    }

    ASTNode *ParseChar()
    {
        if(End())
            return nullptr;
        CP cp = Char();

        if(cp == '^')
        {
            Advance();
            return NewASTNode(ASTNode::Begin);
        }

        if(cp == '$')
        {
            Advance();
            return NewASTNode(ASTNode::End);
        }

        if(cp == '&')
        {
            Advance();
            return NewASTNode(ASTNode::Save);
        }

        switch(cp)
        {
        case '[':
        case ']':
        case '(':
        case ')':
        case '{':
        case '}':
        case '+':
        case '*':
        case '?':
        case '|':
        case '<':
        case '>':
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
            case 'a': cp = '\a'; break;
            case 'b': cp = '\b'; break;
            case 'f': cp = '\f'; break;
            case 'n': cp = '\n'; break;
            case 'r': cp = '\r'; break;
            case 't': cp = '\t'; break;
            case 'v': cp = '\v'; break;
            case '0': cp = '\0'; break;
            case '[':
            case ']':
            case '(':
            case ')':
            case '{':
            case '}':
            case '+':
            case '*':
            case '?':
            case '.':
            case '|':
            case '<':
            case '>':
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
            if(!ret)
                Error();
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
        
        if(AdvanceIf('<'))
        {
            
            ErrIfEnd();
            auto fstChNode = ParseChar();
            if(!fstChNode)
                Error();
            
            ErrIfEnd();
            auto sndChNode = ParseChar();
            if(!sndChNode)
            {
                AdvanceOrErr('>');
                switch(fstChNode->codePoint)
                {
                case 'd': return NewASTNode(ASTNode::Digit);
                case 'c': return NewASTNode(ASTNode::Alpha);
                case 'w': return NewASTNode(ASTNode::WordChar);
                case 's': return NewASTNode(ASTNode::Whitespace);
                case 'h': return NewASTNode(ASTNode::HexDigit);
                default:
                    Error();
                }
            }
            
            if(fstChNode->codePoint > sndChNode->codePoint)
                Error();
            
            AdvanceOrErr('>');
            
            ASTNode *ret = NewASTNode(ASTNode::CharRange);
            ret->charRange[0] = fstChNode->codePoint;
            ret->charRange[1] = sndChNode->codePoint;
            return ret;
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
            else if(AdvanceIf('{'))
            {
                SkipBlanks();
                size_t firstNum = ParseSize_t();
                SkipBlanks();
                ASTNode *newNode;

                if(AdvanceIf(','))
                {
                    SkipBlanks();

                    newNode = NewASTNode(ASTNode::RepeatRange);
                    newNode->repeatRange.min = firstNum;
                    newNode->repeatRange.max = ParseSize_t();
                    if(!newNode->repeatRange.max ||
                       newNode->repeatRange.min > newNode->repeatRange.max)
                        Error();
                    newNode->repeatRange.content = last;

                    SkipBlanks();
                }
                else
                {
                    newNode = NewASTNode(ASTNode::Repeat);
                    newNode->repeatCount = firstNum;
                    newNode->repeatContent = last;
                }

                AdvanceOrErr('}');
                last = newNode;
            }
            else
                return last;
        }
    }

    void SkipBlanks()
    {
        while(!End())
        {
            CP cp = Char();
            if(cp != ' ' && cp != '\t')
                break;
            Advance();
        }
    }

    size_t ParseSize_t()
    {
        ErrIfEnd();

        CP cp = Char();
        Advance();
        if(cp == '0')
        {
            CP next = End() ? '\0' : Char();
            if('0' <= next && next <= '9')
                Error();
            return 0;
        }
        else if(cp < '0' || '9' < cp)
            Error();
        size_t ret = cp - '0';

        for(;;)
        {
            if(End()) break;
            CP c = Char();
            if(c < '0' || '9' < c)
                break;
            ret = 10 * ret + (c - '0');
            Advance();
        }

        return ret;
    }

    ASTNode *ParseCat()
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

    ASTNode *ParseRegex()
    {
        ASTNode *last = ParseCat();
        if(!last)
            return nullptr;
        while(AdvanceIf('|'))
        {
            ASTNode *right = ParseCat();
            if(!right)
                Error();
            ASTNode *newNode = NewASTNode(ASTNode::Or);
            newNode->orDest[0] = last;
            newNode->orDest[1] = right;
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

    explicit Program(size_t instCount)
        : nextInst_(0), instCount_(instCount)
    {
        AGZ_ASSERT(instCount);
        insts_ = new Inst<CP>[instCount];
    }

    Program(Self &&moveFrom) noexcept
        : nextInst_(moveFrom.nextInst_),
          instCount_(moveFrom.instCount_),
          insts_(moveFrom.insts_)
    {
        moveFrom.insts_ = nullptr;
    }

    Self &operator=(Self &&moveFrom) noexcept
    {
        if(insts_)
            delete insts_;
        nextInst_ = moveFrom.nextInst_;
        instCount_ = moveFrom.instCount_;
        insts_ = moveFrom.insts_;
        moveFrom.insts_ = nullptr;
        return *this;
    }

    Program(const Self &)         = delete;
    Self &operator=(const Self &) = delete;

    ~Program()
    {
        if(insts_)
        {
            for(size_t i = 0; i < instCount_; ++i)
            {
                switch(insts_[i].op)
                {
                case Inst<CP>::Alter:
                    if(insts_[i].alterDest)
                        delete[] insts_[i].alterDest;
                    break;
                default:
                    break;
                }
            }
            delete[] insts_;
        }
    }

    bool IsAvailable() const
    {
        return insts_ != nullptr;
    }

    size_t GetInstCount() const
    {
        return nextInst_;
    }

    Inst<CP> &operator[](size_t idx)
    {
        AGZ_ASSERT(insts_ && idx < nextInst_);
        return insts_[idx];
    }

    const Inst<CP> &operator[](size_t idx) const
    {
        AGZ_ASSERT(insts_ && idx < nextInst_);
        return insts_[idx];
    }

    Inst<CP> *Emit(const Inst<CP> &inst)
    {
        AGZ_ASSERT(insts_ && nextInst_ < instCount_);
        insts_[nextInst_] = inst;
        return &insts_[nextInst_++];
    }

    Inst<CP> *GetNextPtr() const { return &insts_[nextInst_]; }
    
    void ReinitLastSteps()
    {
        for(size_t i = 0; i < instCount_; ++i)
            insts_[i].lastStep = numeric_limits<size_t>::max();
    }

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
    using BP = list<Inst<CP>**>;

    Program<CP> Compile(const StringView<CS> &regex,
                        size_t *saveSlotCount)
    {
        AGZ_ASSERT(saveSlotCount);

        Parser<CS> parser;
        auto ast = parser.Parse(regex);

        Program<CP> ret(CountInst(ast) + 1);
        prog_ = &ret;
        saveSlotCount_ = 0;

        auto bps = Generate(ast);
        FillBP(bps, prog_->Emit(MakeInst(Inst<CP>::Match)));

        *saveSlotCount = saveSlotCount_;

        return move(ret);
    }

private:

    Program<CP> *prog_ = nullptr;
    size_t saveSlotCount_ = 0;

    using I = Inst<CP>;
    I MakeInst(typename I::Type type) { return I { type }; }

    void FillBP(BP &bps, Inst<CP> *val)
    {
        for(auto p : bps)
            *p = val;
        bps.clear();
    }

    static size_t CountInst(ASTNode *n)
    {
        AGZ_ASSERT(n);
        switch(n->type)
        {
        case ASTNode::Begin:
        case ASTNode::End:
        case ASTNode::Dot:
        case ASTNode::Char:
        case ASTNode::CharRange:
        case ASTNode::Digit:
        case ASTNode::Alpha:
        case ASTNode::WordChar:
        case ASTNode::Whitespace:
        case ASTNode::HexDigit:
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
                return ret + n->alterCount;
            }
        case ASTNode::Or:
            return 2 + CountInst(n->orDest[0]) + CountInst(n->orDest[1]);
        case ASTNode::Star:
            return 2 + CountInst(n->starDest);
        case ASTNode::Plus:
            return 1 + CountInst(n->plusDest);
        case ASTNode::Ques:
            return 1 + CountInst(n->quesDest);
        case ASTNode::Repeat:
            return n->repeatCount * CountInst(n->repeatContent);
        case ASTNode::RepeatRange:
            return n->repeatRange.max * CountInst(n->repeatRange.content)
                + (n->repeatRange.max == n->repeatRange.min ? 0 : 1);
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
        case ASTNode::CharRange:
            return GenerateCharRange(node);
        case ASTNode::Digit:
            prog_->Emit(MakeInst(I::Digit));
            return { };
        case ASTNode::Alpha:
            prog_->Emit(MakeInst(I::Alpha));
            return { };
        case ASTNode::WordChar:
            prog_->Emit(MakeInst(I::WordChar));
            return { };
        case ASTNode::Whitespace:
            prog_->Emit(MakeInst(I::Whitespace));
            return { };
        case ASTNode::HexDigit:
            prog_->Emit(MakeInst(I::HexDigit));
            return { };
        case ASTNode::Save:
            prog_->Emit(MakeInst(I::Save))->saveSlot = saveSlotCount_++;
            return { };
        case ASTNode::Cat:
        {
            auto bps = Generate(node->catDest[0]);
            FillBP(bps, prog_->GetNextPtr());
            return Generate(node->catDest[1]);
        }
        case ASTNode::Alter:
            return GenerateAlter(node);
        case ASTNode::Or:
            return GenerateOr(node);
        case ASTNode::Star:
            return GenerateStar(node);
        case ASTNode::Plus:
            return GeneratePlus(node);
        case ASTNode::Ques:
            return GenerateQues(node);
        case ASTNode::Repeat:
            return GenerateRepeat(node);
        case ASTNode::RepeatRange:
            return GenerateRepeatRange(node);
        }
        Unreachable();
    }
    
    BP GenerateCharRange(ASTNode *node)
    {
        AGZ_ASSERT(node && node->type == ASTNode::CharRange);
        
        auto cr = prog_->Emit(MakeInst(I::CharRange));
        cr->charRange[0] = node->charRange[0];
        cr->charRange[1] = node->charRange[1];
        
        return { };
    }

    BP GenerateAlter(ASTNode *node)
    {
        AGZ_ASSERT(node && node->type == ASTNode::Alter);

        size_t alterCount = node->alterCount;

        auto alter = prog_->Emit(MakeInst(I::Alter));
        alter->alterCount = alterCount;
        alter->alterDest = new Inst<CP>*[alterCount];

        auto alterDest = alter->alterDest;
        auto alterDestNode = node->alterDest;
        AGZ_ASSERT(alterDestNode);

        alterDest[0] = prog_->GetNextPtr();
        BP ret(Generate(alterDestNode->dest));

        for(size_t i = 1; i < alterCount; ++i)
        {
            alterDestNode = alterDestNode->next;
            AGZ_ASSERT(alterDestNode);

            ret.push_back(&prog_->Emit(MakeInst(I::Jump))->jumpDest);
            alterDest[i] = prog_->GetNextPtr();
            ret.splice(ret.end(), Generate(alterDestNode->dest));
        }

        return move(ret);
    }

    BP GenerateOr(ASTNode *node)
    {
        AGZ_ASSERT(node && node->type == ASTNode::Or);

        auto branch = prog_->Emit(MakeInst(I::Branch));

        branch->branchDest[0] = prog_->GetNextPtr();
        auto bps = Generate(node->orDest[0]);
        bps.push_back(&prog_->Emit(MakeInst(I::Jump))->jumpDest);

        branch->branchDest[1] = prog_->GetNextPtr();
        bps.splice(bps.end(), Generate(node->orDest[1]));

        return move(bps);
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
        return move(ret);
    }

    BP GenerateRepeat(ASTNode *node)
    {
        AGZ_ASSERT(node && node->type == ASTNode::Repeat);

        if(!node->repeatCount)
            return { };
            
        BP bps = Generate(node->repeatContent);
        for(size_t i = 1; i < node->repeatCount; ++i)
        {
            FillBP(bps, prog_->GetNextPtr());
            bps = Generate(node->repeatContent);
        }

        return move(bps);
    }

    BP GenerateRepeatRange(ASTNode *node)
    {
        AGZ_ASSERT(node && node->type == ASTNode::RepeatRange);

        if(!node->repeatRange.max)
            return { };

        auto range = &node->repeatRange;
        BP bps;
        for(size_t i = 0; i < range->min; ++i)
        {
            FillBP(bps, prog_->GetNextPtr());
            bps = Generate(range->content);
        }

        size_t remain = range->max - range->min;
        if(!remain)
            return move(bps);

        FillBP(bps, prog_->GetNextPtr());
        AGZ_ASSERT(bps.empty());

        auto alter = prog_->Emit(MakeInst(I::Alter));
        alter->alterCount = remain + 1;
        alter->alterDest = new Inst<CP>*[remain + 1];

        for(size_t i = 0; i < remain; ++i)
        {
            FillBP(bps, prog_->GetNextPtr());
            alter->alterDest[i] = prog_->GetNextPtr();
            bps = Generate(range->content);
        }

        bps.push_back(&alter->alterDest[remain]);

        return move(bps);
    }
};

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
        storage_ = arena_.Malloc<SaveSlotsStorage>();
        storage_->refs = 1;
        for(size_t i = 0; i < slotCount_; ++i)
            storage_->slots[i] = numeric_limits<size_t>::max();
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
    Thread(Inst<CP> *pc, SaveSlots &&saveSlots, size_t startIdx)
        : pc(pc), saveSlots(move(saveSlots)), startIdx(startIdx)
    {

    }

    Inst<CP> *pc;
    SaveSlots saveSlots;
    
    size_t startIdx;
};

template<typename CS>
class Machine
{
public:

    using CU = typename CS::CodeUnit;
    using CP = typename CS::CodePoint;

    using Interval = pair<size_t, size_t>;

    explicit Machine(const StringView<CS> &regex)
        : slotCount_(0), regex_(regex),
          cpr_(nullptr), matchedStart_(0), matchedEnd_(0)
    {
        
    }

    optional<vector<size_t>>
        Match(const StringView<CS> &dst) const
    {
        if(!prog_.IsAvailable())
        {
            prog_ = Compiler<CS>().Compile(regex_, &slotCount_);
            regex_ = String<CS>();
        }
        auto ret = Run<true, true>(dst);
        return ret.has_value() ? make_optional(move(ret.value().second))
                               : nullopt;
    }

    optional<pair<pair<size_t, size_t>,
        vector<size_t>>>
        Search(const StringView<CS> &dst) const
    {
        if(!prog_.IsAvailable())
        {
            prog_ = Compiler<CS>().Compile(regex_, &slotCount_);
            regex_ = String<CS>();
        }
        return Run<false, false>(dst);
    }

private:

    using It = typename CS::Iterator;
    using CPR = CodePointRange<CS>;

    mutable Program<CP> prog_;
    mutable size_t slotCount_;
    mutable String<CS> regex_;
    
    mutable CPR *cpr_;
    mutable It cur_;
    
    mutable size_t matchedStart_, matchedEnd_;
    mutable optional<SaveSlots> matchedSaveSlots_;
    
    void SetMatched(SaveSlots &&saves) const
    {
        matchedSaveSlots_.emplace(move(saves));
    }

    void AddThread(vector<Thread<CP>> &thds, size_t curStep,
                   Inst<CP> *pc, SaveSlots &&saves, size_t startIdx) const
    {
        if(pc->lastStep == curStep)
            return;
        switch(pc->op)
        {
        case Inst<CP>::Begin:
            if(cpr_->begin() != cur_)
                return;
            AddThread(thds, curStep,
                      pc + 1, move(saves),
                      startIdx);
            break;
        case Inst<CP>::End:
            if(cur_ != cpr_->end())
                return;
            AddThread(thds, curStep,
                      pc + 1, move(saves),
                      startIdx);
            break;
        case Inst<CP>::Jump:
            AddThread(thds, curStep,
                      pc->jumpDest, move(saves),
                      startIdx);
            break;
        case Inst<CP>::Branch:
            AddThread(thds, curStep,
                      pc->branchDest[0], SaveSlots(saves),
                      startIdx);
            AddThread(thds, curStep,
                      pc->branchDest[1], move(saves),
                      startIdx);
            break;
        case Inst<CP>::Save:
            saves.Set(pc->saveSlot, cpr_->CodeUnitIndex(cur_));
            AddThread(thds, curStep,
                      pc + 1, move(saves),
                      startIdx);
            break;
        case Inst<CP>::Alter:
            for(size_t i = 0; i < pc->alterCount; ++i)
            {
                AddThread(thds, curStep,
                          pc->alterDest[i], SaveSlots(saves),
                          startIdx);
            }
            break;
        default:
            thds.push_back(Thread<CP>(pc, move(saves), startIdx));
            break;
        }
    }
    
    void AddThreadWithPC(vector<Thread<CP>> &thds,
                         size_t cpIdx, Inst<CP> *pc,
                         Thread<CP> *oriTh) const
    {
        auto oldCur = cur_;
        ++cur_;
        AddThread(thds, cpIdx,
                  pc, move(oriTh->saveSlots),
                  oriTh->startIdx);
        cur_ = oldCur;
    }
    
    template<bool AnchorBegin, bool AnchorEnd>
    optional<pair<Interval, vector<size_t>>>
        Run(const StringView<CS> &str) const
    {
        AGZ_ASSERT(prog_.IsAvailable());
        
        using namespace std;

        size_t saveAllocSize = SaveSlots::AllocSize(slotCount_);
        FixedSizedArena<> saveSlotsArena(
            saveAllocSize, 16 * saveAllocSize);
        
        prog_.ReinitLastSteps();
        vector<Thread<CP>> rdyThds, newThds;
        rdyThds.reserve(prog_.GetInstCount());
        newThds.reserve(prog_.GetInstCount());
        
        CPR cpr = str.CodePoints();
        cpr_ = &cpr;
        cur_ = cpr.begin();
        
        if constexpr(AnchorBegin)
        {
            AddThread(rdyThds, 0, &prog_[0],
                      SaveSlots(slotCount_, saveSlotsArena),
                      0);
        }
        
        size_t cpIdx = 0;
        for(; cur_ != cpr.end(); ++cur_, ++cpIdx)
        {
            if constexpr(!AnchorBegin)
            {
                AddThread(rdyThds, cpIdx, &prog_[0],
                          SaveSlots(slotCount_, saveSlotsArena),
                          cpIdx);
            }
            
            CP cp = *cur_;
            
            if constexpr(AnchorBegin)
            {
                if(rdyThds.empty())
                    break;
            }
            
            for(size_t i = 0; i < rdyThds.size(); ++i)
            {
                Thread<CP> *th = &rdyThds[i];
                Inst<CP> *pc = th->pc;
                
                switch(pc->op)
                {
                case Inst<CP>::Dot:
                    AddThreadWithPC(newThds, cpIdx, pc + 1, th);
                    break;
                case Inst<CP>::Char:
                    if(pc->codePoint == cp)
                        AddThreadWithPC(newThds, cpIdx, pc + 1, th);
                    break;
                case Inst<CP>::CharRange:
                    if(pc->charRange[0] <= cp && cp <= pc->charRange[1])
                        AddThreadWithPC(newThds, cpIdx, pc + 1, th);
                    break;
                case Inst<CP>::Digit:
                    if(IsUnicodeDigit(cp))
                        AddThreadWithPC(newThds, cpIdx, pc + 1, th);
                    break;
                case Inst<CP>::Alpha:
                    if(IsUnicodeAlpha(cp))
                        AddThreadWithPC(newThds, cpIdx, pc + 1, th);
                    break;
                case Inst<CP>::WordChar:
                    if(IsUnicodeAlnum(cp) || cp == '_')
                        AddThreadWithPC(newThds, cpIdx, pc + 1, th);
                    break;
                case Inst<CP>::Whitespace:
                    if(IsUnicodeWhitespace(cp))
                        AddThreadWithPC(newThds, cpIdx, pc + 1, th);
                    break;
                case Inst<CP>::HexDigit:
                    if(IsUnicodeHexDigit(cp))
                        AddThreadWithPC(newThds, cpIdx, pc + 1, th);
                    break;
                case Inst<CP>::Match:
                    if constexpr(!AnchorEnd)
                    {
                        matchedSaveSlots_.emplace(move(th->saveSlots));
                        matchedStart_     = th->startIdx;
                        matchedEnd_       = cpr.CodeUnitIndex(cur_);
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
            if(th.pc->op == Inst<CP>::Match)
            {
                matchedSaveSlots_.emplace(move(th.saveSlots));
                matchedStart_ = th.startIdx;
                matchedEnd_   = str.Length();
                break;
            }
        }
        
        if(matchedSaveSlots_)
        {
            vector<size_t> slots(slotCount_);
            for(size_t i = 0; i < slotCount_; ++i)
                slots[i] = matchedSaveSlots_.value().Get(i);
            return make_pair(Interval{ matchedStart_, matchedEnd_ },
                             move(slots));
        }
        
        return nullopt;
    }
};

AGZ_NS_END(AGZ::VMEngineImpl)
