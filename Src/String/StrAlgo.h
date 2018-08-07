#pragma once

#include <limits>
#include <type_traits>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ::StrAlgo)

// Boyer–Moore–Horspool algorithm.
// See https://en.wikipedia.org/wiki/Boyer–Moore–Horspool_algorithm
template<size_t AlignBytes>
const unsigned char *BoyerMooreHorspool(const unsigned char *beg, const unsigned char *end,
                                        const unsigned char *pbeg, const unsigned char *pend)
{
    AGZ_ASSERT(beg <= end && pbeg <= pend);

    // Align to AlignByte + alignOffset:
    //     ALIGN_TO(p - alignOffset, AlignByte) + alignOffset
    size_t alignOffset = reinterpret_cast<size_t>(beg) & (AlignBytes - 1);
    size_t alignFactor = (AlignBytes - 1) - alignOffset;
    size_t len = end - beg, pLen = pend - pbeg;
    if(len < pLen)
        return end;
    if(!pLen)
        return beg;

    // Skip table
    // IMPROVE: Consider static + thread_local to save stack space?
    size_t T[std::numeric_limits<unsigned char>::max()];

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

AGZ_NS_END(AGZ::StrAlgo)
