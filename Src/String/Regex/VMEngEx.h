#pragma once

#include <type_traits>
#include <vector>

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
    bool isClass;
    CP ch[2];
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

    const uint32_t *GetRelativeOffset(size_t instIdx) const
    {
        AGZ_ASSERT(instIdx + 1 < instCount_);
        return &insts_[instIdx + 1].instArrUnit[0];
    }
};

AGZ_NS_END(AGZ::VMEngExImpl)
