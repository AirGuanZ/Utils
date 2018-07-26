#pragma once

#include <string>

#include "../Common.h"
#include "../Result/Option.h"

AGZ_NS_BEG(AGZ::Unicode)

// See https://en.wikipedia.org/wiki/UTF-8
AGZ_INLINE std::string CodePoint2UTF8(char32_t codePoint)
{
    std::string ret;
    if(codePoint <= 0x7f)
    {
        ret.resize(1);
        ret[0] = char(codePoint);
    }
    else if(codePoint <= 0x7ff)
    {
        ret.resize(2);
        ret[0] = char((3 << 6) | (codePoint >> 6));
        ret[1] = char((1 << 7) | (codePoint & 0x3f));
    }
    else if(codePoint <= 0xffff)
    {
        ret.resize(3);
        ret[0] = char((7 << 5) | (codePoint >> 12));
        ret[1] = char((1 << 7) | ((codePoint >> 6) & 0x3f));
        ret[2] = char((1 << 7) | (codePoint & 0x3f));
    }
    else if(codePoint < 0x10ffff)
    {
        ret.resize(4);
        ret[0] = char((15 << 4) | (codePoint >> 18));
        ret[1] = char((1 << 7) | ((codePoint >> 12) & 0x3f));
        ret[2] = char((1 << 7) | ((codePoint >> 6) & 0x3f));
        ret[3] = char((1 << 7) | (codePoint & 0x3f));
    }
    return std::move(ret);
}

// Return None when beg >= end or there is an decoding error
// IMPROVE: ugly branches and repeated similar code segments
AGZ_INLINE Option<char32_t> NextCodePointInUTF8(const char *beg, const char *end,
                                            size_t *skipBytes)
{
#define SET_SKIP(N) \
    do { \
        if(skipBytes) *skipBytes = (N); \
    } while(0)

    if(beg >= end)
    {
        SET_SKIP(0);
        return None<char32_t>();
    }

    char fst = *beg++;

    // 1 bytes
    if((fst & (1 << 7)) == 0)
    {
        SET_SKIP(1);
        return Some(char32_t(fst));
    }

    auto fetch6Bits = [](char ch) -> Option<std::uint32_t>
    {
        if((ch & (3 << 6)) != (1 << 7))
            return None<uint32_t>();
        return Some(uint32_t(ch & 0x3f));
    };

    // 4 bytes
    if((fst & (0x1f << 3)) == (0x1e << 3))
    {
        auto oHigh6 = fetch6Bits(*beg++);
        if(oHigh6.IsNone())
        {
            SET_SKIP(1);
            return None<char32_t>();
        }
        auto oMid6 = fetch6Bits(*beg++);
        if(oMid6.IsNone())
        {
            SET_SKIP(1);
            return None<char32_t>();
        }
        auto oLow6 = fetch6Bits(*beg);
        if(oLow6.IsNone())
        {
            SET_SKIP(1);
            return None<char32_t>();
        }
        SET_SKIP(4);
        return Some(char32_t(((0x7 & fst) << 18) |
                             (oHigh6.Unwrap() << 12) |
                             (oMid6.Unwrap() << 6) |
                             (oLow6.Unwrap())));
    }

    // 3 bytes
    if((fst & (0xf << 4)) == (0xe << 3))
    {
        auto oHigh6 = fetch6Bits(*beg++);
        if(oHigh6.IsNone())
        {
            SET_SKIP(1);
            return None<char32_t>();
        }
        auto oLow6 = fetch6Bits(*beg);
        if(oLow6.IsNone())
        {
            SET_SKIP(1);
            return None<char32_t>();
        }
        SET_SKIP(3);
        return Some(char32_t(((0xf & fst) << 12) |
                             (oHigh6.Unwrap() << 6) |
                             (oLow6.Unwrap())));
    }

    // 2 bytes
    if((fst & (0x7 << 5)) == (0x6 << 5))
    {
        auto snd6 = fetch6Bits(*beg);
        if(snd6.IsNone())
        {
            SET_SKIP(1);
            return None<char32_t>();
        }
        SET_SKIP(2);
        return Some(char32_t(((0x1f & fst) << 6) |
                             (snd6.Unwrap())));
    }

    // Unknown prefix
    SET_SKIP(1);
    return None<char32_t>();

#undef SET_SKIP
}

AGZ_NS_END(AGZ::Unicode)
