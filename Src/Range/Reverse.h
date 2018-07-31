#pragma once

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

template<typename It>
class ReverseIterator
{
    It it;

public:

    using iterator_category = typename It::iterator_category;
    using value_type        = typename It::value_type;
    using difference_type   = typename It::difference_type;
    using pointer           = typename It::pointer;
    using reference         = typename It::reference;

    ReverseIterator(It it)
        : it(std::move(it))
    {

    }

    value_type operator*() const
    {
        return *--It(it);
    }

    const pointer operator->() const
    {
        return (--It(it)).operator->();
    }

    ReverseIterator &operator++()
    {
        --it;
        return *this;
    }

    ReverseIterator operator++(int)
    {
        auto ret = *this;
        --it;
        return ret;
    }

    ReverseIterator &operator--()
    {
        ++it;
        return *this;
    }

    ReverseIterator operator--(int)
    {
        auto ret = *this;
        ++it;
        return *this;
    }

    ReverseIterator &operator+=(difference_type n)
    {
        it -= n;
        return *this;
    }

    friend ReverseIterator operator+(const ReverseIterator &i, difference_type n)
    {
        return ReverseIterator(i.it - n);
    }

    friend ReverseIterator operator+(difference_type n, const ReverseIterator &i)
    {
        return i - n;
    }

    ReverseIterator &operator-=(difference_type n)
    {
        it += n;
        return *this;
    }

    friend ReverseIterator operator-(ReverseIterator i, difference_type n)
    {
        return ReverseIterator(i.it + n);
    }

    friend difference_type operator-(const ReverseIterator &b, const ReverseIterator &a)
    {
        return a - b;
    }

    value_type operator[](difference_type n) const
    {
        return *(*this + n);
    }

    friend bool operator==(const ReverseIterator &a, const ReverseIterator &b)
    {
        return a.it == b.it;
    }

    friend bool operator!=(const ReverseIterator &a, const ReverseIterator &b)
    {
        return !(a == b);
    }

    friend bool operator<(const ReverseIterator &a, const ReverseIterator &b)
    {
        return b.it < a.it;
    }

    friend bool operator>(const ReverseIterator &a, const ReverseIterator &b)
    {
        return b < a;
    }

    friend bool operator<=(const ReverseIterator &a, const ReverseIterator &b)
    {
        return !(a > b);
    }

    friend bool operator>=(const ReverseIterator &a, const ReverseIterator &b)
    {
        return !(a < b);
    }
};

namespace RangeAux
{
    template<typename R>
    class ReverseImpl
    {
        R range_;

    public:

        using Iterator = ReverseIterator<typename R::Iterator>;

        ReverseImpl(R range)
            : range_(std::move(range))
        {

        }

        Iterator begin() const
        {
            return Iterator(std::end(range_));
        }

        Iterator end() const
        {
            return Iterator(std::begin(range_));
        }
    };

    struct ReverseRHS { };
}

inline RangeAux::ReverseRHS Reverse()
{
    return RangeAux::ReverseRHS { };
}

template<typename R>
auto operator|(R &&range, RangeAux::ReverseRHS rhs)
{
    using RT = RangeAux::ReverseImpl<
        std::remove_cv_t<std::remove_reference_t<R>>>;
    return RT(std::forward<R>(range));
}

AGZ_NS_END(AGZ)
