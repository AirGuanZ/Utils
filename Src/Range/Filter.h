#pragma once

#include <iterator>

#include "../Misc/Common.h"
#include "Iterator.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<typename R, typename F,
            std::enable_if_t<
                std::is_same_v<
                    decltype(std::declval<F>()(
                        *std::declval<typename R::Iterator>())),
                    bool>,
                int> = 0>
    class FilterImpl
    {
        R range_;
        mutable F func_;

    public:

        class Iterator
        {
            using InIt = typename R::Iterator;

            InIt it, end;
            F *f;

        public:

            // Doesn't support RandomAccessIterator
            using iterator_category =
                std::conditional_t<
                    IsBidirectionalIterator<InIt>,
                    std::bidirectional_iterator_tag,
                    typename InIt::iterator_category>;

            using value_type        = typename InIt::value_type;
            using difference_type   = typename InIt::difference_type;
            using pointer           = typename InIt::pointer;
            using reference         = typename InIt::reference;

            Iterator(InIt _it, InIt _end, F *f)
                : it(std::move(_it)), end(_end), f(f)
            {
                while(it != end && !(*f)(*it))
                    ++it;
            }

            value_type operator*() const
            {
                return *it;
            }

            pointer operator->() const
            {
                return it.operator->();
            }

            Iterator &operator++()
            {
                while(it != end && !(*f)(*++it))
                    ;
                return *this;
            }

            Iterator operator++(int)
            {
                auto ret = *this;
                ++*this;
                return ret;
            }

            Iterator &operator--()
            {
                while(!(*f)(*--it))
                    ;
                return *this;
            }

            Iterator operator--(int)
            {
                auto ret = *this;
                --*this;
                return ret;
            }

            bool operator==(const Iterator &rhs) const
            {
                AGZ_ASSERT(f == rhs.f && end == rhs.end);
                return it == rhs.it;
            }

            bool operator!=(const Iterator &rhs) const
            {
                return !(*this == rhs);
            }
        };

        FilterImpl(R range, F f)
            : range_(std::move(range)), func_(std::move(f))
        {

        }

        Iterator begin() const
        {
            return Iterator(std::begin(range_), std::end(range_), &func_);
        }

        Iterator end() const
        {
            return Iterator(std::end(range_), std::end(range_), &func_);
        }
    };

    template<typename F>
    struct FilterRHS { F f; };
}

template<typename F>
RangeAux::FilterRHS<F> Filter(F f)
{
    return RangeAux::FilterRHS<F>{ std::move(f) };
}

template<typename R, typename F>
auto operator|(R &&range, RangeAux::FilterRHS<F> &&rhs)
{
    return RangeAux::FilterImpl<remove_rcv_t<R>,
                                    std::remove_reference_t<F>>
            (std::forward<R>(range), std::move(rhs.f));
}

AGZ_NS_END(AGZ)
