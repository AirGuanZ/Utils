#pragma once

#include <iterator>

#include "../Misc/Common.h"
#include "Iterator.h"
#include "Transform.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<typename R, typename F>
    class MapImpl
    {
        R range_;
        mutable F func_;

        using InIt = GetIteratorType<R>;

    public:

        class Iterator
        {
            friend class MapImpl;

            using _value_type =
                decltype(std::declval<F>()(std::declval<
                        typename std::iterator_traits<InIt>::value_type>()));

            InIt it;
            F *f;

            class ptr_t
            {
                _value_type val;

            public:

                explicit ptr_t(_value_type val)
                    : val(std::move(val))
                {

                }

                const _value_type &operator*() const
                {
                    return val;
                }

                _value_type &operator*()
                {
                    return val;
                }

                const _value_type *operator->() const
                {
                    return &val;
                }

                _value_type *operator->()
                {
                    return &val;
                }
            };

        public:

            using iterator_category =
                typename std::iterator_traits<InIt>::iterator_category;
            using value_type        = _value_type;
            using difference_type   =
                typename std::iterator_traits<InIt>::difference_type;
            using pointer           = ptr_t;
            using reference         = value_type&;

            Iterator(InIt it, F *f)
                : it(std::move(it)), f(f)
            {

            }

            value_type operator*() const
            {
                return (*f)(*it);
            }

            pointer operator->() const
            {
                return pointer((*f)(*it));
            }

            Iterator &operator++()
            {
                ++it;
                return *this;
            }

            Iterator operator++(int)
            {
                auto ret = *this;
                ++it;
                return ret;
            }

            Iterator &operator--()
            {
                --it;
                return *this;
            }

            Iterator operator--(int)
            {
                auto ret = *this;
                --it;
                return ret;
            }

            Iterator &operator+=(difference_type n)
            {
                it += n;
                return *this;
            }

            Iterator &operator-=(difference_type n)
            {
                it -= n;
                return *this;
            }

            value_type operator[](difference_type n) const
            {
                return (*f)(*it);
            }

            friend Iterator operator+(Iterator i, difference_type n)
            {
                return Iterator(i.it + n, i.f);
            }

            friend Iterator operator+(difference_type n, Iterator i)
            {
                return i + n;
            }

            friend Iterator operator-(Iterator i, difference_type n)
            {
                return Iterator(i.it - n, i.f);
            }

            friend difference_type operator-(const Iterator &b,
                                             const Iterator &a)
            {
                return b.it - a.it;
            }

            friend bool operator==(const Iterator &a, const Iterator &b)
            {
                AGZ_ASSERT(a.f == b.f);
                return a.it == b.it;
            }

            friend bool operator!=(const Iterator &a, const Iterator &b)
            {
                return !(a == b);
            }

            friend bool operator<(const Iterator &a, const Iterator &b)
            {
                AGZ_ASSERT(a.f == b.f);
                return a.it < b.it;
            }

            friend bool operator>(const Iterator &a, const Iterator &b)
            {
                return b < a;
            }

            friend bool operator<=(const Iterator &a, const Iterator &b)
            {
                return !(a > b);
            }

            friend bool operator>=(const Iterator &a, const Iterator &b)
            {
                return !(a < b);
            }
        };

        MapImpl(R &&range, F &&f)
            : range_(std::move(range)), func_(std::move(f))
        {

        }

        Iterator begin() const
        {
            return Iterator(std::begin(range_), &func_);
        }

        Iterator end() const
        {
            return Iterator(std::end(range_), &func_);
        }
    };

    template<typename F>
    struct MapTrait
    {
        template<typename R>
        using Impl = MapImpl<R, F>;
    };
}

template<typename F>
auto Map(F f)
{
    return RangeAux::TransformWrapper<
            RangeAux::MapTrait<F>, F>(std::move(f));
}

AGZ_NS_END(AGZ)
