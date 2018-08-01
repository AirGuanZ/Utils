#pragma once

#include <limits>
#include <type_traits>

#include "../Misc/Common.h"
#include "../Range/Iterator.h"
#include "../Range/Reverse.h"

AGZ_NS_BEG(AGZ::StrAlgo)

template<typename It>
std::enable_if_t<IsRandomAccessIterator<It>, bool>
StartsWith(It beg1, It end1, It beg2, It end2);

template<typename It>
std::enable_if_t<IsRandomAccessIterator<It>, bool>
EndsWith(It beg1, It end1, It beg2, It end2);

template<typename It>
std::enable_if_t<IsRandomAccessIterator<It>, size_t>
Find(It beg1, It end1, It beg2, It end2);

template<typename It>
std::enable_if_t<IsRandomAccessIterator<It>, size_t>
RFind(It beg1, It end1, It beg2, It end2);

enum class CompareResult { Less, Equal, Greater };

template<typename It>
std::enable_if_t<IsRandomAccessIterator<It>, CompareResult>
Comp(It beg1, It end1, It beg2, It end2);

AGZ_NS_END(AGZ::StrAlgo)

AGZ_NS_BEG(AGZ::StrAlgo)

constexpr size_t NPOS = std::numeric_limits<size_t>::max();

template<typename It>
std::enable_if_t<IsRandomAccessIterator<It>, bool>
StartsWith(It beg1, It end1, It beg2, It end2)
{
    AGZ_ASSERT(beg1 <= end1 && beg2 <= end2);
    size_t len1 = end1 - beg1, len2 = end2 - beg2;
    if(len1 < len2)
        return false;
    for(size_t i = 0; i < len2; ++i)
    {
        if(*beg1++ != *beg2++)
            return false;
    }
    return true;
}

template<typename It>
std::enable_if_t<IsRandomAccessIterator<It>, bool>
EndsWith(It beg1, It end1, It beg2, It end2)
{
    AGZ_ASSERT(beg1 <= end1 && beg2 <= end2);
    size_t len1 = end1 - beg1, len2 = end2 - beg2;
    if(len1 < len2)
        return false;
    beg1 += len1 - len2;
    for(size_t i = 0; i < len2; ++i)
    {
        if(*beg1++ != *beg2++)
            return false;;
    }
    return true;
}

template<typename It>
std::enable_if_t<IsRandomAccessIterator<It>, size_t>
Find(It beg1, It end1, It beg2, It end2)
{
    // IMPROVE: Brute-force

    AGZ_ASSERT(beg1 <= end1 && beg2 <= end2);

    size_t len1 = end1 - beg1, len2 = end2 - beg2;
    if(len1 < len2)
        return NPOS;
    end1 = beg1 + (len1 - len2 + 1);

    size_t ret = 0;
    while(beg1 < end1)
    {
        bool found = true;
        It b1 = beg1, b2 = beg2;
        for(size_t i = 0; i < len2; ++i)
        {
            if(*b1++ != *b2++)
            {
                found = false;
                break;
            }
        }
        if(found)
            return ret;
        ++beg1, ++ret;
    }

    return NPOS;
}

template<typename It>
std::enable_if_t<IsRandomAccessIterator<It>, size_t>
RFind(It beg1, It end1, It beg2, It end2)
{
    AGZ_ASSERT(beg1 <= end1 && beg2 <= end2);

    using RI = ReverseIterator<It>;
    size_t rev_rt = Find(RI(end1), RI(beg1), RI(end2), RI(beg2));
    if(rev_rt == NPOS)
        return NPOS;
    return static_cast<size_t>(end1 - beg1) - rev_rt
         - static_cast<size_t>(end2 - beg2);
}

template<typename It>
std::enable_if_t<IsRandomAccessIterator<It>, CompareResult>
Comp(It beg1, It end1, It beg2, It end2)
{
    AGZ_ASSERT(beg1 <= end1 && beg2 <= end2);

    while(beg1 < end1 && beg2 < end2)
    {
        auto c1 = *beg1++, c2 = *beg2++;
        if(c1 < c2) return CompareResult::Less;
        if(c1 > c2) return CompareResult::Greater;
    }

    if(beg1 == end1)
    {
        if(beg2 == end2)
            return CompareResult::Equal;
        return CompareResult::Less;
    }

    return beg1 == end1 ? (beg2 == end2 ? CompareResult::Equal
                                        : CompareResult::Less)
                        : CompareResult::Greater;
}

AGZ_NS_END(AGZ::StrAlgo)
