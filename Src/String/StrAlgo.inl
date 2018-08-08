#pragma once

#include <algorithm>
#include <limits>
#include <type_traits>
#include <vector>

AGZ_NS_BEG(AGZ::StrImpl::StrAlgo)

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
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                'U', 'V', 'W', 'X', 'Y', 'Z'
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

AGZ_NS_END(AGZ::StrImpl::StrAlgo)
