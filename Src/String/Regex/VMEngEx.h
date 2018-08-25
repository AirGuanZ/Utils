#pragma once

#include <type_traits>

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

enum class InstType
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
struct Inst
{
    union
    {
        struct
        {
            InstType type;

            union
            {
                struct { CP codePoint;      } dataChar;
                struct { CP fst, lst;       } dataCharRange;
                struct { CP codePoint;      } dataCharExprSingle;
                struct { CP fst, lst;       } dataCharExprRange;
                struct { size_t saveSlot;   } dataSave;
                struct { size_t alterCount; } dataAlter;
                struct { Inst<CP> *dest;    } dataJump;
                struct { Inst<CP> *fstDest; } dataBranch;
            };

            size_t lastStep;
        };

        Inst<CP> *instArrUnit[2];
    };
};

enum class ASTType
{
    Begin,
    End,

    Save,

    Cat,
    CharClass,
    Or,

    CharSingle,
    CharAny,
    CharRange,
    CharDecDigit,
    CharHexDigit,
    CharAlpha,
    CharWordChar,
    CharWhitespace,

    CharExprSingle,
    CharExprAny,
    CharExprRange,
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

struct ASTNode
{
    ASTType type;
};

AGZ_NS_END(AGZ::VMEngExImpl)
