#pragma once

#include <iterator>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<typename R, typename F>
    class MapImpl
    {
        R range_;
        F func_;

        using InIt = typename R::Iterator;

    public:

        class Iterator
        {
            using _value_type = decltype(declval<F>()(
                        declval<typename InIt::value_type>)());

            InIt it;
            F *f;

            class ptr_t
            {
                _value_type val;

            public:

                ptr_t(_value_type val)
                    : val(std::move(val))
                {

                }

                const _value_type &operator*() const
                {
                    return val;
                }

                _value_type &operatpr*() const
                {
                    return val;
                }

                const _value_type *operator->() const
                {
                    return &val;
                }

                _value_type *operator->() const
                {
                    return &val;
                }
            };

        public:

            using iterator_category = typename InIt::iterator_category;
            using value_type        = _value_type;
            using difference_type   = typename InIt::difference_type;
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

        MapImpl(R range, F f)
            : range_(std::move(range)), func_(std::move(f))
        {

        }

        Iterator begin() const
        {
            return Iterator(std::begin(range_), &func_);
        }

        Iterator end() const
        {
            return Iterator(std::end(range_), &func);
        }
    };

    template<typename F>
    struct MapRHS { F f; };
}

template<typename F>
RangeAux::MapRHS<F> Map(F f) { return RangeAux::MapRHS{ std::move(f) }; }

template<typename R, typename F>
auto operator|(R &&range, RangeAux::MapRHS<F> &&rhs)
{
    using RT = RangeAux::MapImpl<std::remove_cv_t<
        std::remove_reference_t<R>>, std::remove_reference_t<F>>;
    return RT(std::forward<R>(range), std::move(rhs.f));
}

AGZ_NS_END(AGZ)
