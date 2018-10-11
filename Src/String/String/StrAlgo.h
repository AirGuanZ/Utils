#pragma once

#include <algorithm>
#include <limits>
#include <type_traits>
#include <vector>

#include "../../Misc/Exception.h"
#include "String.h"

AGZ_NS_BEG(AGZ::StrAlgo)

// Boyer–Moore–Horspool algorithm.
// See https://en.wikipedia.org/wiki/Boyer–Moore–Horspool_algorithm
template<size_t AlignBytes>
const unsigned char *BoyerMooreHorspool(const unsigned char *beg,
                                        const unsigned char *end,
                                        const unsigned char *pbeg,
                                        const unsigned char *pend)
{
    AGZ_ASSERT(beg <= end && pbeg <= pend);

    // Align to AlignByte + alignOffset:
    //     ALIGN_TO(p - alignOffset, AlignByte) + alignOffset
    //
    // alignOffset and alignFactor are meaningful only when
    // the value of beg is not aligned as AlignBytes.
    // If this requirement can be guaranteed, these two variables
    // must be 0 and can be eliminated.
    //
    // Believe that the compiler will do sufficient constant propagation
    // when AlignBytes == 0
    //
    // :)
    size_t alignOffset = reinterpret_cast<size_t>(beg) & (AlignBytes - 1);
    size_t alignFactor = (AlignBytes - 1) - alignOffset;
    size_t len = end - beg, pLen = pend - pbeg;
    if(len < pLen)
        return end;
    if(!pLen)
        return beg;

    // Skip table
    // IMPROVE: Consider static + thread_local to save stack space?
    size_t T[std::numeric_limits<unsigned char>::max() + 1];

    // Preprocessing
    size_t pLenM1 = pLen - 1;
    for(auto &v : T)
        v = pLen;
    for(size_t i = 0; i < pLenM1; ++i)
        T[pbeg[i]] = pLenM1 - i;

    // Matching
    size_t skip = 0, skipEnd = len - pLenM1;
    while(skip < skipEnd)
    {
        auto i = pLenM1, j = skip + i;
        while(beg[j] == pbeg[i])
        {
            if(!i)
                return beg + skip;
            --i, --j;
        }
        skip = ((skip + T[beg[skip + pLenM1]] + alignFactor)
                & ~(AlignBytes - 1)) + alignOffset;
    }

    return end;
}

// Boyer-Moore-Horspool ALgorithm
template<typename CU>
const CU *FindSubPattern(const CU *beg, const CU *end,
                         const CU *pbeg, const CU *pend)
{
    return reinterpret_cast<const CU*>(
            BoyerMooreHorspool<sizeof(CU)>(
                reinterpret_cast<const unsigned char*>(beg),
                reinterpret_cast<const unsigned char*>(end),
                reinterpret_cast<const unsigned char*>(pbeg),
                reinterpret_cast<const unsigned char*>(pend)));
}

enum class CompareResult { Greater, Equal, Less };

template<typename CU>
CompareResult Compare(const CU *lhs, const CU *rhs, size_t lLen, size_t rLen)
{
    size_t i = 0, minLen = std::min(lLen, rLen);
    while(i < minLen)
    {
        if(lhs[i] < rhs[i])
            return CompareResult::Less;
        if(lhs[i] > rhs[i])
            return CompareResult::Greater;
        ++i;
    }
    return i == lLen ? (i == rLen ? CompareResult::Equal :
                                    CompareResult::Less)
                     : CompareResult::Greater;
}

template<typename T, typename CS,
         std::enable_if_t<std::is_integral_v<T>, int> = 0>
    String<CS> Int2Str(T v, unsigned int base)
{
    // IMPROVE

    AGZ_ASSERT(2 <= base && base <= 36);
    std::vector<typename CS::CodeUnit> cus;

    [[maybe_unused]]
    bool neg = false;

    // Handle negative sign
    if constexpr(std::is_signed_v<T>)
    {
        // Corner case ignored: v == numeric_limits<T>::min()
        if(v < 0)
        {
            neg = true;
            v = ~v + 1;
        }
    }

    if(!v)
        cus.push_back('0');
    else
    {
        while(v)
        {
            static const char chs[36] =
            {
                '0', '1', '2', '3', '4',
                '5', '6', '7', '8', '9',
                'A', 'B', 'C', 'D', 'E',
                'F', 'G', 'H', 'I', 'J',
                'K', 'L', 'M', 'N', 'O',
                'P', 'Q', 'R', 'S', 'T',
                'U', 'V', 'W', 'X', 'Y',
                'Z'
            };
            auto d = v % base;
            v      = v / base;
            cus.push_back(chs[d]);
        }
    }

    if constexpr(std::is_signed_v<T>)
    {
        if(neg)
            cus.push_back('-');
    }

    std::reverse(std::begin(cus), std::end(cus));
    return String<CS>(cus.data(), cus.size());
}

// < 10    : digit
// [10, 36): alpha
// 128     : whitespaces
// 255     : others
inline const unsigned char DIGIT_CHAR_VALUE_TABLE[256] =
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 128,
    128, 128, 128, 128, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 128, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 0,   1,
    2,   3,   4,   5,   6,   7,   8,   9,   255, 255,
    255, 255, 255, 255, 255, 10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
    25,  26,  27,  28,  29,  30,  31,  32,  33,  34,
    35,  255, 255, 255, 255, 255, 255, 10,  11,  12,
    13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
    23,  24,  25,  26,  27,  28,  29,  30,  31,  32,
    33,  34,  35,  255, 255, 255, 255, 255,

    // There are 128 redundant values to enable indexing
    // this table directly with any integral type with 1 byte size
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255
};

AGZ_FORCEINLINE unsigned char DTV(char32_t ucp)
{
    return static_cast<uint32_t>(ucp < 256) ?
           DIGIT_CHAR_VALUE_TABLE[ucp] : 255;
}

AGZ_FORCEINLINE bool IsUnicodeDigit(char32_t ucp)
{
    return '0' <= ucp && ucp <= '9';
}

AGZ_FORCEINLINE bool IsUnicodeHexDigit(char32_t ucp)
{
    return DTV(ucp) < 16;
}

AGZ_FORCEINLINE bool IsUnicodeAlpha(char32_t ucp)
{
    auto v = DTV(ucp);
    return 10 <= v && v < 36;
}

AGZ_FORCEINLINE bool IsUnicodeAlnum(char32_t ucp)
{
    return DTV(ucp) < 36;
}

AGZ_FORCEINLINE bool IsUnicodeWhitespace(char32_t ucp)
{
    return DTV(ucp) == 128;
}

template<typename T, typename CS>
struct Str2IntImpl
{
    static T Convert(const StringView<CS> &str, unsigned int base)
    {
        AGZ_ASSERT(base <= 36);

        auto chars = str.CodePoints();
        auto cur = chars.begin(), end = chars.end();

        [[maybe_unused]]
        bool neg = false;

        auto begcp = *cur;
        if(begcp == '+')
            ++cur;
        else if constexpr(std::is_signed_v<T>)
        {
            if(begcp == '-')
            {
                neg = true;
                ++cur;
            }
        }

        auto c = cur;
        if(cur == end || (*c++ == '0' && c != end))
            throw ArgumentException("Parsing error in Str2Int");

        T ret = T(0);
        while(cur != end)
        {
            auto cp = *cur++;
            if(cp < 0 || cp >= 128 || DIGIT_CHAR_VALUE_TABLE[cp] >= base)
                throw ArgumentException("Parsing error in Str2Int");
            ret = base * ret + DIGIT_CHAR_VALUE_TABLE[cp];
        }

        return neg ? -ret : ret;
    }
};

template<typename T, typename CS,
            std::enable_if_t<std::is_integral_v<T>, int> = 0>
    T Str2Int(const StringView<CS> &str, unsigned int base)
{
    return Str2IntImpl<T, CS>::Convert(str, base);
}

AGZ_NS_END(AGZ::StrAlgo)
