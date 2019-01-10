#pragma once

#include <iterator>

#include "../Misc/Common.h"
#include "Iterator.h"
#include "Transform.h"

namespace AGZ {

namespace RangeAux
{
    template<typename R, typename F>
    class FilterImpl
    {
        R range_;
        mutable F func_;

    public:

        class Iterator
        {
            using InIt = GetIteratorType<R>;

            InIt it, end;
            F *f;

        public:

            // Doesn't support RandomAccessIterator
            using iterator_category =
                std::conditional_t<
                    IsBidirectionalIterator<InIt>,
                    std::bidirectional_iterator_tag,
                    typename std::iterator_traits<InIt>::iterator_category>;

            using value_type        =
                typename std::iterator_traits<InIt>::value_type;
            using difference_type   =
                typename std::iterator_traits<InIt>::difference_type;
            using pointer           =
                typename std::iterator_traits<InIt>::pointer;
            using reference         =
                typename std::iterator_traits<InIt>::reference;

            Iterator(InIt _it, InIt _end, F *f)
                : it(std::move(_it)), end(std::move(_end)), f(f)
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
                while(++it != end && !(*f)(*it))
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

        FilterImpl(R &&range, F &&f)
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
    struct FilterTrait
    {
        template<typename R>
        using Impl = FilterImpl<R, F>;
    };
}

/**
 * @brief 丢弃range中不满足给定谓词的元素
 */
template<typename F>
auto Filter(F f)
{
    return RangeAux::TransformWrapper<
            RangeAux::FilterTrait<F>, F>(std::move(f));
}

} // namespace AGZ
