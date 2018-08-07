#pragma once

#include <type_traits>

#include "../Misc/Common.h"
#include "../Range/Iterator.h"
#include "Charset.h"

AGZ_NS_BEG(AGZ)

template<typename T>
class UTF8Iterator
{
    const T *cur;

    using Self = UTF8Iterator<T>;

public:

    using iterator_category = std::bidirectional_iterator_tag;
    using value_type        = char32_t;
    using difference_type   = std::make_signed_t<size_t>;
    using pointer           = ValuePointer<char32_t>;
    using reference         = char32_t&;

    explicit UTF8Iterator(const T *cur);

    char32_t operator*() const;
    pointer operator->() const;

    UTF8Iterator &operator++();
    UTF8Iterator operator++(int);
    UTF8Iterator &operator--();
    UTF8Iterator operator--(int);

    bool operator==(const Self &rhs) const;
    bool operator!=(const Self &rhs) const;
};

template<typename T>
class UTF8Core
{
public:

    using Iterator  = UTF8Iterator<T>;
    using CodePoint = char32_t;
    using CodeUnit  = T;

    static std::string Name() { return "UTF-8"; }

    static constexpr size_t MaxCUInCP = 4;

    static size_t CUInCP(CodePoint cp);

    static size_t CP2CU(CodePoint cp, CodeUnit *cu);
    static size_t CU2CP(const CodeUnit *cu, CodePoint *cp);

    static char32_t ToUnicode(CodePoint cp) { return cp; }
    static CodePoint FromUnicode(char32_t cp) { return cp; }

    static const CodeUnit *NextCodePoint(const CodeUnit *cur);
    static const CodeUnit *LastCodePoint(const CodeUnit *cur);
};

template<typename T = char>
using UTF8 = Charset<UTF8Core<T>>;

template<typename T>
size_t UTF8Core<T>::CUInCP(CodePoint cp)
{
    if(cp <= 0x7f)     return 1;
    if(cp <= 0x7ff)    return 2;
    if(cp <= 0xffff)   return 3;
    if(cp <= 0x10ffff) return 4;
    return 0;
}

template<typename T>
size_t UTF8Core<T>::CP2CU(CodePoint cp, CodeUnit *cu)
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

template<typename T>
size_t UTF8Core<T>::CU2CP(const CodeUnit *cu, CodePoint *cp)
{
    AGZ_ASSERT(cu && cp && cu_num);

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
    if((fst & 0b11100000) == 0b11000000)
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

template<typename T>
const typename UTF8Core<T>::CodeUnit *
UTF8Core<T>::NextCodePoint(const CodeUnit *cur)
{
    T fst = *cur;
    if(!(fst & 0b10000000))
        return cur + 1;
    if((fst & 0b11100000) == 0b11000000)
    {
        if((++*cur & 0b11000000) != 0b10000000)
            throw EncodingException("Advancing in invalid UTF-8 sequence");
        return cur + 1;
    }
    if((fst & 0b11110000) == 0b11100000)
    {
        if((++*cur & 0b11000000) != 0b10000000 ||
           (++*cur & 0b11000000) != 0b10000000)
            throw EncodingException("Advancing in invalid UTF-8 sequence");
        return cur + 1;
    }
    if((fst & 0b11111000) == 0b11110000)
    {
        if((++*cur & 0b11000000) != 0b10000000 ||
           (++*cur & 0b11000000) != 0b10000000 ||
           (++*cur & 0b11000000) != 0b10000000)
           throw EncodingException("Advancing in invalid UTF-8 sequence");
        return cur + 1;
    }
    throw EncodingException("Advancing in invalid UTF-8 sequence");
}

template<typename T>
const typename UTF8Core<T>::CodeUnit *
UTF8Core<T>::LastCodePoint(const CodeUnit *cur)
{
    while((*--cur) & 0b11000000 == 0b10000000)
        ;
    return cur;
}

template<typename T>
UTF8Iterator<T>::UTF8Iterator(const T *cur)
    : cur(cur)
{

}

template<typename T>
char32_t UTF8Iterator<T>::operator*() const
{
    char32_t ret;
    if(!UTF8Core<T>::CU2CP(cur, &ret))
        throw EncodingException("Dereferencing invalid UTF-8 iterator");
    return ret;
}

template<typename T>
typename UTF8Iterator<T>::pointer UTF8Iterator<T>::operator->() const
{
    return pointer(**this);
}

template<typename T>
UTF8Iterator<T> &UTF8Iterator<T>::operator++()
{
    cur = UTF8Core<T>::NextCodePoint(cur);
    return *this;
}

template<typename T>
UTF8Iterator<T> UTF8Iterator<T>::operator++(int)
{
    auto ret = *this;
    ++*this;
    return *this;
}

template<typename T>
UTF8Iterator<T> &UTF8Iterator<T>::operator--()
{
    cur = UTF8Core<T>::LastCodePoint(cur);
    return *this;
}

template<typename T>
UTF8Iterator<T> UTF8Iterator<T>::operator--(int)
{
    auto ret = *this;
    --*this;
    return *this;
}

template<typename T>
bool UTF8Iterator<T>::operator==(const UTF8Iterator<T> &rhs) const
{
    return cur == rhs.cur;
}

template<typename T>
bool UTF8Iterator<T>::operator!=(const UTF8Iterator<T> &rhs) const
{
    return !(*this == rhs);
}

AGZ_NS_END(AGZ)
