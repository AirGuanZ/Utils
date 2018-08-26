#pragma once

#include <type_traits>

#include "../Alloc/FixedSizedArena.h"
#include "../../Misc/Common.h"
#include "../String/StrAlgo.h"
#include "../String/String.h"

/*
    ab                    concatenation
    a|b                   alternative
    [a-c0-9def]           char class
    a+                    one or more
    a*                    zero or more
    a?                    zero or one
    ^                     beginning
    $                     end
    &                     save point
    .                     any character
    a{m}                  m times
    a{m, n}               m to n times
    @{[a-c] & [b-d] & !c} char expression
    \d                    dec digit
    \c                    a-z A-Z
    \w                    alnum _
    \s                    whitespace
    \h                    hex digit

Grammar:

    Regex    := Cat | Cat | ... | Cat
    Cat      := Fac Fac ... Fac
    Fac      := Fac*
                Fac+
                Fac?
                Fac{m}
                Fac{m, n}
                Core
    Core     := (Regex)
                Class
                Char
                @{CharExpr}
    Class    := [ClassMem ClassMem ... ClassMem]
    ClassMem := Char-Char
                Char
    CharExpr := AndExpr | AndExpr | ... | AndExpr
    AndExpr  := FacExpr & FacExpr & ... & FacExpr
    FacExpr  := Char
                Class
                !FacExpr
                (CharExpr)
*/

AGZ_NS_BEG(AGZ::VMEngExImpl)

using namespace StrImpl::StrAlgo;

enum class InstType : uint32_t
{
    Begin,                            // String beginning
    End,                              // String end

    Save,                             // Create a save point

    Alter,                            // Multi-threaded branch
    Jump,                             // Unconditioned jump
    Branch,                           // Split to two threads

    Match,                            // Succeed

    CharSingle = 7,                   // Specified character
    CharAny,                          // Any character
    CharRange,                        // Character range
    CharDecDigit,                     // Decimal digit
    CharHexDigit,                     // Hexadecimal digit
    CharAlpha,                        // Alpha character
    CharWord,                         // Word(Alpha/digit/underscore) character
    CharWhitespace,                   // Whitespace character

    CharExprSingle = 32 | CharSingle, // Single character -> bool
    CharExprAny,                      // Any character    -> true
    CharExprRange,                    // Character range  -> bool
    CharExprDecDigit,                 
    CharExprHexDigit,                 
    CharExprAlpha,                    
    CharExprWordChar,                 
    CharExprWhitespace,               

    CharExprAnd,                      // bool, bool       -> bool
    CharExprOr,                       // bool, bool       -> bool
    CharExprNot,                      // bool             -> bool

    CharExprEnd,                      // End of bool expression
};

constexpr InstType Char2Expr(InstType type)
{
    return static_cast<InstType>(
        static_cast<std::underlying_type_t<InstType>>(type) | 32);
}

template<typename CP>
union Inst
{
    static_assert(sizeof(CP) <= sizeof(uint32_t));

    struct
    {
        InstType type;

        union
        {
            struct { CP codePoint;      } dataChar;
            struct { CP fst, lst;       } dataCharRange;
            struct { CP codePoint;      } dataCharExprSingle;
            struct { CP fst, lst;       } dataCharExprRange;
            struct { uint32_t slot;     } dataSave;
            struct { uint32_t count;    } dataAlter;
            struct { int32_t offset;    } dataJump;
            struct { int32_t fstOffset; } dataBranch;
        };

        uint32_t lastStep;
    };

    int32_t instArrUnit[4];
};

static_assert(sizeof(Inst<char>)     == 4 * sizeof(int32_t));
static_assert(sizeof(Inst<char16_t>) == 4 * sizeof(int32_t));
static_assert(sizeof(Inst<char32_t>) == 4 * sizeof(int32_t));

enum class ASTType
{
    Begin,
    End,

    Save,

    Cat,
    Or,

    Star,
    Plus,
    Ques,
    Repeat,

    CharSingle,
    CharAny,
    CharClass,
    CharDecDigit,
    CharHexDigit,
    CharAlpha,
    CharWordChar,
    CharWhitespace,

    CharExprSingle,
    CharExprAny,
    CharExprClass,
    CharExprDecDigit,
    CharExprHexDigit,
    CharExprAlpha,
    CharExprWordChar,
    CharExprWhitespace,

    CharExprAnd,
    CharExprOr,
    CharExprNot,

    CharExprEnd,
};

template<typename CP>
struct ClassMemNode
{
    ClassMemNode *next;
    bool isRange;
    CP fst, snd;
};

template<typename CP>
struct ASTNode
{
    ASTType type;

    union
    {
        struct { size_t slot;                             } dataSave;

        struct { ASTNode *dest[2];                        } dataCat;
        struct { ASTNode *dest[2];                        } dataOr;

        struct { ASTNode *dest;                           } dataStar;
        struct { ASTNode *dest;                           } dataPlus;
        struct { ASTNode *dest;                           } dataQues;
        struct { ASTNode *dest; uint32_t fst, lst;        } dataRepeat;

        struct { CP codePoint;                            } dataCharSingle;
        struct { ClassMemNode<CP> *mems; uint32_t memCnt; } dataCharClass;
        struct { CP codePoint;                            } dataCharExprSingle;
        struct { ClassMemNode<CP> *mems; uint32_t memCnt; } dataCharExprClass;
    };
};

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
        moveFrom.insts_        = nullptr;
        moveFrom.instCapacity_ = 0;
        moveFrom.instCount_    = 0;

        moveFrom.relativeOffsetCount_ = 0;
    }

    Self &operator=(Self &&moveFrom) noexcept
    {
        Release();

        insts_        = moveFrom.insts_;
        instCapacity_ = moveFrom.instCapacity_;
        instCount_    = moveFrom.instCount_;

        relativeOffsetCount_ = moveFrom.relativeOffsetCount_;

        moveFrom.insts_        = nullptr;
        moveFrom.instCapacity_ = 0;
        moveFrom.instCount_    = 0;

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
class Parser
{
public:

    using CP = typename CS::CodePoint;
    using Node = ASTNode<CS>;

    Parser()
        : astNodeArena_(32)
    {
    }

    Node *Parse(const StringView<CS> &src)
    {
        CodePointRange<CS> cpr = src.CodePoints();
        cur_ = cpr.begin();
        end_ = cpr.end();

        Node *ret = ParseRegex();

        if(!End())
            Error();

        return ret;
    }

private:

    /*
        Class    := [ClassMem ClassMem ... ClassMem]
        ClassMem := Char-Char
                    Char
        CharExpr := AndExpr | AndExpr | ... | AndExpr
        AndExpr  := FacExpr & FacExpr & ... & FacExpr
        FacExpr  := Char
                    Class
                    !FacExpr
                    (CharExpr)
    */

    /*
        Regex := Cat | Cat | ... | Cat
    */
    Node *ParseRegex()
    {
        Node *last = ParseCat();
        if(!last)
            return nullptr;

        while(AdvanceIf('|'))
        {
            Node *right = ParseCat();
            if(!right)
                Error();
            Node *newNode = NewNode(ASTType::Or);
            newNode->dataOr.dest[0] = last;
            newNode->dataOr.dest[1] = right;
            last = newNode;
        }

        return last;
    }

    /*
        Cat := Fac Fac ... Fac
    */
    Node *ParseCat()
    {
        Node *last = ParseFac();
        if(!last)
            return nullptr;

        Node *right;
        while((right = ParseFac()))
        {
            Node *newNode = NewNode(ASTType::Cat);
            newNode->dataCat.dest[0] = last;
            newNode->dataCat.dest[1] = right;
            last = newNode;
        }

        return last;
    }

    /*
        Fac := Fac*
               Fac+
               Fac?
               Fac{m}
               Fac{m, n}
               Core
    */
    Node *ParseFac()
    {
        Node *core = ParseCore();
        return core ? ParseFacRest(core) : nullptr;
    }

    Node *ParseFacRest(Node *last)
    {
        if(End())
            return last;
        switch(Cur())
        {
        case '*':
            return ParseFacStar(last);
        case '+':
            return ParseFacPlus(last);
        case '?':
            return ParseFacQues(last);
        case '{':
            return ParseFacRepeat(last);
        default:
            return last;
        }
    }

    Node *ParseFacStar(Node *last)
    {
        Node *newLast = NewNode(ASTType::Star);
        newLast->dataStar = last;
        return ParseFacRest(newLast);
    }

    Node *ParseFacPlus(Node *last)
    {
        Node *newLast = NewNode(ASTType::Plus);
        newLast->dataPlus = last;
        return ParseFacRest(newLast);
    }

    Node *ParseFacQues(Node *last)
    {
        Node *newLast = NewNode(ASTType::Ques);
        newLast->dataQues = last;
        return ParseFacRest(newLast);
    }

    Node *ParseFacRepeat(Node *last)
    {
        SkipBlanks();
        uint32_t fstNum = ParseUInt();
        SkipBlanks();

        Node *newNode = NewNode(ASTType::Repeat);
        newNode->dataRepeat.dest = last;
        newNode->dataRepeat.fst  = fstNum;

        if(AdvanceIf(','))
        {
            SkipBlanks();
            uint32_t sndNum = ParseUInt();
            SkipBlanks();

            if(!sndNum || fstNum > sndNum)
                Error();
            newNode->dataRepeat.lst = sndNum;
        }
        else
            newNode->dataRepeat.lst = fstNum;

        AdvanceOrErr('}');
        return ParseFacRest(newNode);
    }

    /*
        Core := (Regex)
                Class
                Char
                @{CharExpr}
    */
    Node *ParseCore()
    {
        if(AdvanceIf('('))
        {
            Node *ret = ParseRegex();
            if(!ret)
                Error();
            AdvanceOrErr(')');
            return ret;
        }

        if(Match('['))
            return ParseCharClass();

        if(AdvanceIf('@'))
            return ParseCharExpr();

        return ParseChar();
    }

    Node *ParseCharClass()
    {
        AGZ_ASSERT(!End() && Cur() == '[');
        Advance();

        enum
        {
            WaitingForFirst,
            WaitingForHyphen,
            WaitingForSecond
        } state = WaitingForFirst;

        CP fst, snd;
        Node *ret = NewNode(ASTType::CharClass);
        ret->dataCharClass.memCnt = 0;
        ret->dataCharClass.mems   = nullptr;

        for(;;)
        {
            ErrIfEnd();

            switch(state)
            {
            case WaitingForFirst:
                if(AdvanceIf(']'))
                    goto END_OF_CLASS_MEMS;
                fst = NextClassChar();
                state = WaitingForHyphen;
                break;

            case WaitingForHyphen:
                if(AdvanceIf('-'))
                    state = WaitingForSecond;
                else
                {
                    state = WaitingForFirst;

                    auto n = classMemNodeArena_.Malloc();
                    n->next = ret->dataCharClass.mems;
                    n->isRange = false;
                    n->fst = fst;

                    ret->dataCharClass.mems = n;
                    ++ret->dataCharClass.mems;
                }
                break;

            case WaitingForSecond:
            {
                state = WaitingForFirst;

                auto n = classMemNodeArena_.Malloc();
                n->next = ret->dataCharClass.mems;
                n->isRange = true;
                n->fst = fst;
                n->snd = NextClassChar();

                ret->dataCharClass.mems = n;
                ++ret->dataCharClass.mems;
            }
                break;
            }
        }

    END_OF_CLASS_MEMS:
        return ret;
    }

    CP NextClassChar()
    {
        // TODO
        return 0;
    }

    Node *ParseCharExpr()
    {
        // TODO
        return nullptr;
    }

    Node *ParseChar()
    {
        // TODO
        return nullptr;
    }

    void SkipBlanks()
    {
        while(!End())
        {
            if(StrImpl::StrAlgo::IsUnicodeWhitespace(Cur()))
                break;
            Advance();
        }
    }

    uint32_t ParseUInt()
    {
        ErrIfEnd();

        CP cp = Cur();
        Advance();
        if(cp == '0')
        {
            CP next = End() ? '\0' : Cur();
            if('0' <= next && next <= '9')
                Error();
            return 0;
        }
        if(cp < '0' || '9' < cp)
            Error();
        uint32_t ret = cp - '0';

        for(;;)
        {
            if(End()) break;
            CP c = Cur();
            if(c < '0' || '9' < c)
                break;
            ret = 10 * ret + (c - '0');
            Advance();
        }

        return ret;
    }

    Node *NewNode(ASTType type)
    {
        Node *ret = astNodeArena_.Malloc();
        ret->type = type;
        return ret;
    }

    [[noreturn]] static AGZ_FORCEINLINE void Error()
    {
        throw ArgumentException("Syntax error in regular expression");
    }

    CP Cur() const
    {
        AGZ_ASSERT(!End());
        return *cur_;
    }

    bool End() const
    {
        return cur_ == end_;
    }

    void ErrIfEnd() const
    {
        if(End())
            Error();
    }

    bool Match(CP cp)
    {
        return !End() && Cur() == cp;
    }

    void Advance()
    {
        AGZ_ASSERT(!End());
        ++cur_;
    }

    bool AdvanceIf(CP cp)
    {
        if(Match(cp))
        {
            Advance();
            return true;
        }
        return false;
    }

    void AdvanceOrErr(CP cp)
    {
        if(!AdvanceIf(cp))
            Error();
    }

    SmallObjArena<Node> astNodeArena_;
    SmallObjArena<ClassMemNode<CP>> classMemNodeArena_;
    typename CodePointRange<CS>::Iterator cur_, end_;
};

AGZ_NS_END(AGZ::VMEngExImpl)
