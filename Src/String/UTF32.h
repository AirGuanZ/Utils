#pragma once

#include "../Misc/Common.h"
#include "CharSet.h"

AGZ_NS_BEG(AGZ)

namespace UTF32Aux
{
    template<typename T>
    class UTF32Iterator
    {
        const T *cur;

    public:

        using Self = UTF32Iterator<T>;

        // IMPROVE: can be random_access_iterator
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = char32_t;
        using difference_type   = std::make_signed_t<size_t>;
        using pointer           = char32_t*;
        using reference         = char32_t&;

        UTF32Iterator(const T *cur, const T *end) : cur(cur) { }

        char32_t operator*() const { return *cur; }

        const char32_t *operator->() const { return cur; }

        Self &operator++() { ++cur; return *this; }

        Self operator++(int) { auto ret = *this; ++cur; return ret; }

        Self &operator--() { --cur; return *this; }

        Self operator--(int) { auto ret = *this; --cur; return ret; }

        bool operator==(const Self &rhs) const { return cur == rhs.cur; }

        bool operator!=(const Self &rhs) const { return !(*this == rhs); }
    };
}

template<typename T>
class UTF32Core
{
public:

    static_assert(sizeof(T) >= 4);

    using Iterator  = UTF32Aux::UTF32Iterator<T>;
    using CodePoint = char32_t;
    using CodeUnit  = T;

    static std::string Name() { return "UTF-32"; }

    static constexpr size_t MaxCUInCP = 1;

    static size_t CP2CU(CodePoint cp, CodeUnit *cu)
    {
        cu[0] = cp;
        return 1;
    }

    static size_t CU2CP(const CodeUnit *cu, CodePoint *cp, size_t cu_num)
    {
        AGZ_ASSERT(cu && cp && cu_num > 0);
        *cp = cu[0];
        return 1;
    }

    static char32_t ToUnicode(CodePoint cp) { return cp; }

    static CodePoint FromUnicode(char32_t cp) { return cp; }
};

template<typename T = char32_t>
using UTF32 = CharSet<UTF32Core<T>>;

AGZ_NS_END(AGZ)
