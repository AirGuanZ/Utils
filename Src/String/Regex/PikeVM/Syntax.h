#pragma once

#include <optional>

#include "../../../Alloc/FixedSizedArena.h"
#include "../../../Misc/Common.h"
#include "../../String/StrAlgo.h"
#include "../../String/String.h"

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

AGZ_NS_BEG(AGZ::PikeVM)

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

    CharExpr,

    CharExprAnd,
    CharExprOr,
    CharExprNot,
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
    using Self = ASTNode<CP>;

    ASTType type;

    union
    {
        struct { size_t slot;                             } dataSave;

        struct { Self *dest[2];                           } dataCat;
        struct { Self *dest[2];                           } dataOr;

        struct { Self *dest;                              } dataStar;
        struct { Self *dest;                              } dataPlus;
        struct { Self *dest;                              } dataQues;
        struct { Self *dest; uint32_t fst, lst;           } dataRepeat;

        struct { CP codePoint;                            } dataCharSingle;
        struct { ClassMemNode<CP> *mems; uint32_t memCnt; } dataCharClass;

        struct { Self *expr; } dataCharExpr;
        struct { Self *left, *right;                      } dataCharExprOr;
        struct { Self *left, *right;                      } dataCharExprAnd;
        struct { Self *dest;                              } dataCharExprNot;
    };
};

template<typename CS>
class Parser
{
public:

    using CP = typename CS::CodePoint;
    using Node = ASTNode<CP>;

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
        AGZ_ASSERT(Match('*'));
        Advance();
        Node *newLast = NewNode(ASTType::Star);
        newLast->dataStar.dest = last;
        return ParseFacRest(newLast);
    }

    Node *ParseFacPlus(Node *last)
    {
        AGZ_ASSERT(Match('+'));
        Advance();
        Node *newLast = NewNode(ASTType::Plus);
        newLast->dataPlus.dest = last;
        return ParseFacRest(newLast);
    }

    Node *ParseFacQues(Node *last)
    {
        AGZ_ASSERT(Match('?'));
        Advance();
        Node *newLast = NewNode(ASTType::Ques);
        newLast->dataQues.dest = last;
        return ParseFacRest(newLast);
    }

    Node *ParseFacRepeat(Node *last)
    {
        AGZ_ASSERT(Match('{'));
        Advance();

        SkipBlanks();
        uint32_t fstNum = ParseUInt();
        SkipBlanks();

        Node *newNode = NewNode(ASTType::Repeat);
        newNode->dataRepeat.dest = last;
        newNode->dataRepeat.fst = fstNum;

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
        {
            if(!fstNum)
                Error();
            newNode->dataRepeat.lst = fstNum;
        }

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
        {
            AdvanceOrErr('{');
            auto ret = NewNode(ASTType::CharExpr);
            ret->dataCharExpr.expr = ParseCharExpr();
            AdvanceOrErr('}');
            return ret;
        }

        return ParseChar();
    }

    /*
        Class    := [ClassMem ClassMem ... ClassMem]
        ClassMem := Char-Char
        Char
    */
    Node *ParseCharClass()
    {
        AGZ_ASSERT(Match('['));
        Advance();

        enum
        {
            WaitingForFirst,
            WaitingForHyphen,
            WaitingForSecond
        } state = WaitingForFirst;

        CP fst;
        Node *ret = NewNode(ASTType::CharClass);
        ret->dataCharClass.memCnt = 0;
        ret->dataCharClass.mems = nullptr;

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
                    ++ret->dataCharClass.memCnt;
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
                ++ret->dataCharClass.memCnt;
            }
            break;
            }
        }

    END_OF_CLASS_MEMS:
        return ret;
    }

    /*
        转义字符被分为以下两类：
        在当前regex context下由regex语法带来的转义，比如()这种，称为SyntaxEscape
        SyntaxEscape又分了两类环境：
        NormalSyntaxEscape：一般的语法转义，在CharClass以外的环境中生效
        ClassSyntaxEscape：在CharClass中的语法转义
        由于字符本身特性而无法好好写成字面量的转义，比如\a、\n这种，称为NativeEscape
        NativeEscape在任何地方都有效
    */
    std::optional<CP> NativeEscapeChar()
    {
        if(End())
            return std::nullopt;

        CP cp;
        switch(Cur())
        {
        case 'a': cp = '\a'; break;
        case 'b': cp = '\b'; break;
        case 'f': cp = '\f'; break;
        case 'n': cp = '\n'; break;
        case 'r': cp = '\r'; break;
        case 't': cp = '\t'; break;
        case 'v': cp = '\v'; break;
        case '0': cp = '\0'; break;
        case '\\': cp = '\\'; break;
        default:
            return std::nullopt;
        }

        Advance();
        return std::make_optional(cp);
    }

    CP NextClassChar()
    {
        if(Match(']'))
            Error();

        CP cp = Cur();
        Advance();

        if(cp == '\\')
        {
            ErrIfEnd();
            auto nativeEscape = NativeEscapeChar();
            if(nativeEscape.has_value())
                cp = nativeEscape.value();
            else
            {
                CP ncp = Cur();
                switch(ncp)
                {
                case '[':
                case ']':
                case '-':
                    cp = ncp;
                default:
                    Error();
                }
            }
        }

        return cp;
    }

    /*
        CharExpr := AndExpr | AndExpr | ... | AndExpr
    */
    Node *ParseCharExpr()
    {
        Node *last = ParseAndExpr();
        if(!last)
            Error();

        while(AdvanceIf('|'))
        {
            Node *right = ParseAndExpr();
            Node *newNode = NewNode(ASTType::CharExprOr);
            newNode->dataCharExprOr.left = last;
            newNode->dataCharExprOr.right = right;
            last = newNode;
        }

        return last;
    }

    /*
        AndExpr  := FacExpr & FacExpr & ... & FacExpr
        FacExpr  := Char
        Class
        !FacExpr
        (CharExpr)
    */
    Node *ParseAndExpr()
    {
        Node *last = ParseFacExpr();
        if(!last)
            Error();

        while(AdvanceIf('&'))
        {
            Node *right = ParseFacExpr();
            Node *newNode = NewNode(ASTType::CharExprAnd);
            newNode->dataCharExprAnd.left = last;
            newNode->dataCharExprAnd.right = right;
            last = newNode;
        }

        return last;
    }

    Node *ParseFacExpr()
    {
        if(AdvanceIf('!'))
        {
            Node *sub = ParseFacExpr();
            Node *ret = NewNode(ASTType::CharExprNot);
            ret->dataCharExprNot.dest = sub;
            return ret;
        }

        if(AdvanceIf('('))
        {
            Node *ret = ParseCharExpr();
            AdvanceOrErr(')');
            return ret;
        }

        if(Match('['))
            return ParseCharClass();

        Node *ret = ParseChar();
        if(!ret)
            Error();
        return ret;
    }

    Node *ParseChar()
    {
        if(End())
            return nullptr;

        CP cp = Cur();
        if(cp == '^')
        {
            Advance();
            return NewNode(ASTType::Begin);
        }
        if(cp == '$')
        {
            Advance();
            return NewNode(ASTType::End);
        }
        if(cp == '&')
        {
            Advance();
            return NewNode(ASTType::Save);
        }
        if(cp == '.')
        {
            Advance();
            return NewNode(ASTType::CharAny);
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
        case '@':
        case '!':
            return nullptr;
        default:
            break;
        }

        Advance();
        if(cp == '\\')
        {
            auto native = NativeEscapeChar();
            if(native.has_value())
                cp = native.value();
            else
            {
                ErrIfEnd();
                cp = Cur();
                Advance();
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
                case '!':
                case '@':
                case '^':
                case '$':
                case '&':
                case '.':
                    break;
                case 'd':
                    return NewNode(ASTType::CharDecDigit);
                case 'c':
                    return NewNode(ASTType::CharAlpha);
                case 'w':
                    return NewNode(ASTType::CharWordChar);
                case 's':
                    return NewNode(ASTType::CharWhitespace);
                case 'h':
                    return NewNode(ASTType::CharHexDigit);
                default:
                    Error();
                }
            }
        }

        auto ret = NewNode(ASTType::CharSingle);
        ret->dataCharSingle.codePoint = cp;
        return ret;
    }

    void SkipBlanks()
    {
        while(!End())
        {
            if(!StrAlgo::IsUnicodeWhitespace(Cur()))
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
        throw ArgumentException("Ill-formed regular expression");
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

AGZ_NS_END(AGZ::PikeVM)
