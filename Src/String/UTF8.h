#pragma once

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

// En/decoding rules: see https://en.wikipedia.org/wiki/UTF-8
template<typename T = char>
class UTF8
{
public:

    using CodePoint = char32_t;
    using CodeUnit  = T;

    // Maximum count of code units to encode a code point
    static size_t MaxCUInCP() { return 4; }

    // Convert a code point to code units
    // Return count of code units obtained (shall <= MaxCUInCP())
    // Return 0 when cp is invalid
    static size_t CP2CU(CodePoint cp, CodeUnit *cu)
    {
        AGZ_ASSERT(cu);

        if(cp <= 0x7f)
        {
            cu[0] = static_cast<CodeUnit>(cp);
            return 1;
        }

        if(cp <= 0x7ff)
        {
            cu[0] = static_cast<CodeUnit>(0b11000000 | (cp >> 6));
            cu[1] = static_cast<CodeUnit>(0b10000000 | (cp & 0b00111111));
            return 2;
        }

        if(cp <= 0xffff)
        {
            cu[0] = static_cast<CodeUnit>(0b11100000 | (cp >> 12));
            cu[1] = static_cast<CodeUnit>(0b10000000 | ((cp >> 6) & 0b00111111));
            cu[2] = static_cast<CodeUnit>(0b10000000 | (cp & 0b00111111));
            return 3;
        }

        if(cp <= 0x10ffff)
        {
            cu[0] = static_cast<CodeUnit>(0b11110000 | (cp >> 18));
            cu[1] = static_cast<CodeUnit>(0b10000000 | ((cp >> 12) & 0b00111111));
            cu[2] = static_cast<CodeUnit>(0b10000000 | ((cp >> 6) & 0b00111111));
            cu[3] = static_cast<CodeUnit>(0b10000000 | (cp & 0b00111111));
            return 4;
        }

        return 0;
    }

    // Recognize the first code point in a code unit sequence
    // Return count of code units consumed
    // Return 0 when the code uni seq is invalid
    static size_t CU2CP(const CodeUnit *cu, CodePoint *cp)
    {
        AGZ_ASSERT(cu && cp);

        CodeUnit fst = *cu++;

        // 1 bytes
        if(!(fst & 0b10000000))
        {
            *cp = static_cast<CodePoint>(fst);
            return 1;
        }

#define NEXT(C, DST) \
    do {\
        CodeUnit ch = (C); \
        if((ch & 0b11000000) != 0b10000000) \
            return 0; \
        (DST) = ch & 0b00111111; \
    } while(0)

        // 2 bytes
        if((fst & 0b11100000) == 0b1100000)
        {
            CodePoint low;
            NEXT(*cu, low);
            *cp = ((fst & 0b00011111) << 6) | low;
            return 2;
        }

        // 3 bytes
        if((fst & 0b11110000) == 0b11100000)
        {
            CodePoint high, low;
            NEXT(*cu++, high); NEXT(*cu, low);
            *cp = ((fst & 0b00001111) << 12) | (high << 6) | low;
            return 3;
        }

        // 4 bytes
        if((fst & 0b11111000) == 0b11110000)
        {
            CodePoint high, medi, low;
            NEXT(*cu++, high); NEXT(*cu++, medi); NEXT(*cu, low);
            *cp = ((fst & 0b00000111) << 18) | (high << 12) | (medi << 6) | low;
            return 4;
        }

#undef NEXT

        return 0;
    }
};

AGZ_NS_END(AGZ)