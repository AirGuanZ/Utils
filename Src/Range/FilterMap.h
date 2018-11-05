#pragma once

#include <iterator>

#include "../Misc/Common.h"
#include "Iterator.h"
#include "Transform.h"

namespace AGZ {

namespace RangeAux
{
    template<typename R, typename F>
    class FilterMapImpl
    {
        R range_;
        F func_;

    public:

        class Iterator
        {
            using InIt = GetIteratorType<R>;

            InIt it, end;
            const F *f;

        public:

            using iterator_category =
                std::conditional_t<
                    IsBidirectionalIterator<InIt>,
                    std::bidirectional_iterator_tag,
                    typename std::iterator_traits<InIt>::iterator_category>;

            using value_type =
                typename remove_rcv_t<decltype(std::declval<F>()(std::declval<
                    typename std::iterator_traits<InIt>::value_type>()))>::value_type;
            using difference_type =
                typename std::iterator_traits<InIt>::difference_type;
            using pointer = ValuePointer<value_type>;
            using reference = value_type&;

            Iterator(InIt it, InIt end, const F *f)
                : it(std::move(it)), end(std::move(end)), f(f)
            {
                while(this->it != this->end && !(*this->f)(*this->it).has_value())
                    ++this->it;
            }

            value_type operator*() const
            {
                return (*f)(*it).value();
            }

            pointer operator->() const
            {
                return pointer(**this);
            }

            Iterator &operator++()
            {
                while(++it != end && !(*f)(*it).has_value())
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

        FilterMapImpl(R &&range, F &&f)
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
    struct FilterMapTrait
    {
        template<typename R>
        using Impl = FilterMapImpl<R, F>;
    };
}

template<typename F>
auto FilterMap(F f)
{
    return RangeAux::TransformWrapper<
            RangeAux::FilterMapTrait<F>, F>(std::move(f));
}

} // namespace AGZ
