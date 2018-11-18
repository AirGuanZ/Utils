#pragma once

#include <type_traits>

#include "../../../Misc/Common.h"

/**
 * @cond
 */

namespace AGZ::StrImpl::PikeVM {

enum class InstType : uint32_t
{
    Begin,              // String beginning
    End,                // String end

    Save,               // Create a save point

    Alter,              // Multi-threaded branch
    Jump,               // Unconditioned jump
    Branch,             // Split to two threads

    Match,              // Succeed

    CharSingle,         // Specified character
    CharAny,            // Any character
    CharRange,          // Character range
    CharDecDigit,       // Decimal digit
    CharHexDigit,       // Hexadecimal digit
    CharAlpha,          // Alpha character
    CharWordChar,       // Word(Alpha/digit/underscore) character
    CharWhitespace,     // Whitespace character

    CharExprSingle,     // Single character -> bool
    CharExprAny,        // Any character    -> true
    CharExprRange,      // Character range  -> bool
    CharExprDecDigit,
    CharExprHexDigit,
    CharExprAlpha,
    CharExprWordChar,
    CharExprWhitespace,

    CharExprITSTAJ,     // If true  then set true  and jump
    CharExprIFSFAJ,     // If false then set false and jump
    CharExprSetTrue,
    CharExprSetFalse,
    CharExprNot,        // v -> !v

    CharExprEnd,        // If true then continue else exit_thread
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
            struct { CP codePoint;      } dataCharSingle;
            struct { CP fst, lst;       } dataCharRange;
            struct { CP codePoint;      } dataCharExprSingle;
            struct { CP fst, lst;       } dataCharExprRange;
            struct { uint32_t slot;     } dataSave;
            struct { uint32_t count;    } dataAlter;
            struct { int32_t offset;    } dataJump;
            struct { int32_t dest[2];   } dataBranch;
            struct { int32_t offset;    } dataITSTAJ;
            struct { int32_t offset;    } dataIFSFAJ;
        };

        uint32_t lastStep;
    };

    int32_t instArrUnit[4];
};

static_assert(sizeof(Inst<char>)     == 4 * sizeof(int32_t));
static_assert(sizeof(Inst<char16_t>) == 4 * sizeof(int32_t));
static_assert(sizeof(Inst<char32_t>) == 4 * sizeof(int32_t));

} // namespace AGZ::PikeVM

/**
 * @endcond
 */
