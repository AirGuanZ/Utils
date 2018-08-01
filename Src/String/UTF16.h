#pragma once

#include "../Misc/Common.h"
#include "../Range/Iterator.h"
#include "Charset.h"

AGZ_NS_BEG(AGZ)

template<typename T>
class UTF16Core
{
public:

    class Iterator
    {
        using Self = Iterator;

        const T *cur;

    public:

        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = char32_t;
        using difference_type   = std::make_signed_t<size_t>;
        using pointer           = ValuePointer<T>;
        using reference         = char32_t&;

        explicit Iterator(const T *cur);

        value_type operator*() const;

        pointer *operator->() const;

        Self &operator++();

        Self operator++(int);

        Self &operator--();

        Self operator--(int);

        bool operator==(const Self &rhs) const;

        bool operator!=(const Self &rhs) const { return !(*this == rhs);
    };

    static_assert(sizeof(T) >= 2);

    using CodePoint = char32_t;
    using CodeUnit  = T;

    static std::string Name() { return "UTF-16"; }

    static constexpr size_t MaxCUInCP = 2;

    static size_t CUInCP(CodePoint cp);

    static size_t CP2CU(CodePoint cp, CodeUnit *cu);

    static size_t CU2CP(const CodeUnit *cu, CodePoint *cp, size_t cu_num);

    static char32_t ToUnicode(CodePoint cp) { return cp; }

    static CodePoint FromUnicode(char32_t cp) { return cp; }

    static const CodeUnit *LastCodePoint(const CodeUnit *cur);
};

template<typename T = char16_t>
using UTF16 = Charset<UTF16Core<T>>;

template<typename T>
size_t UTF16Core<T>::CUInCP(CodePoint cp)
{
    return cp <= 0xffff ? 1 : 2;
}

template<typename T>
size_t UTF16Core<T>::CP2CU(CodePoint cp, CodeUnit *cu)
{
    if(cp <= 0xd7ff || (0xe000 <= cp && cp <= 0xffff))
    {
        *cu = static_cast<CodeUnit>(cp);
        return 1;
    }

    if(0x10000 <= cp && cp <= 0x10ffff)
    {
        cp -= 0x10000;
        *cu++ = static_cast<CodeUnit>(0xd800 | (cp >> 10));
        *cu = static_cast<CodeUnit>(0xdc00 | (cp & 0x3ff));
        return 2;
    }

    return 0;
}

template<typename T>
size_t UTF16Core<T>::CU2CP(const CodeUnit *cu, CodePoint *cp, size_t cu_num)
{
    AGZ_ASSERT(cu && cp && cu_num);
    char32_t high = static_cast<char32_t>(*cu);

    // 1 code unit
    if(high <= 0xd7ff || (0xe000 <= high && high <= 0xffff))
    {
        *cp = high;
        return 1;
    }

    // 2 code units
    if(0xd800 <= high && high <= 0xdbff)
    {
        char32_t low = static_cast<char32_t>(*++cu);
        if(low <= 0xdfff)
            return 0x10000 + ((high & 0x3ff) << 10) | (low & 0x3ff);
        return 0;
    }

    return 0;
}

template<typename T>
const typename UTF16Core<T>::CodeUnit *
UTF16Core<T>::LastCodePoint(const CodeUnit *cur)
{
    while(0xd800 <= (*--cur) && *cur <= 0xdbff)
        ;
    return cur;
}

template<typename T>
UTF16Iterator<T>::UTF16Iterator(const T *cur)
    : cur(cur)
{
    AGZ_ASSERT(cur);
}

template<typename T>
char32_t UTF16Iterator<T>::operator*() const
{
    char32_t ret;
    if(!UTF16Core<T>::CU2CP(cur, &ret, UTF16Core<T>::MaxCUInCP))
        throw EncodingException("Invalid UTF-16 sequence");
    return ret;
}

template<typename T>
const char32_t *UTF16Iterator<T>::operator->() const
{
    return pointer(**this);
}

template<typename T>
UTF16Iterator<T> &UTF16Iterator<T>::operator++()
{
    cur += UTF16Core<T>::CUInCP(**this);
    return *this;
}

template<typename T>
UTF16Iterator<T> UTF16Iterator<T>::operator++(int)
{
    auto ret = *this;
    ++*this;
    return ret;
}

template<typename T>
UTF16Iterator<T> &UTF16Iterator<T>::operator--()
{
    cur = UTF16Core<T>::LastCodePoint(cur);
    return *this;
}

template<typename T>
UTF16Iterator<T> UTF16Iterator<T>::operator--(int)
{
    auto ret = *this;
    --*this;
    return ret;
}

template<typename T>
bool UTF16Iterator<T>::operator==(const Self &rhs) const
{
    return cur == rhs.cur;
}

AGZ_NS_END(AGZ)
