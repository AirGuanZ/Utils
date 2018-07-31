#pragma once

#include <algorithm>
#include <type_traits>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    class BetweenImpl
    {
        T start_, step_, end_;

    public:

        class Iterator
        {
            T cur, step;

            Iterator(T cur, T step)
                : cur(cur), step(step)
            {

            }

        public:

            friend class BetweenImpl;

            using iterator_category = std::random_access_iterator_tag;
            using value_type        = T;
            using difference_type   = std::make_signed_t<T>;
            using pointer           = T*;
            using reference         = T&;

            T operator*() const
            {
                return cur;
            }

            cnost T *operator->() const
            {
                return &cur;
            }

            Iterator &operator++()
            {
                cur += step;
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
                cur = -= step;
                return *this;
            }

            Iterator operator--(int)
            {
                auto ret = *this;
                --*this;
                return ret;
            }

            Iterator &operator+=(difference_type n)
            {
                cur = += n * step;
                return *this;
            }

            friend Iterator operator+(Iterator i, difference_type n)
            {
                return i += n;
            }

            friend Iterator operator+(difference_type n, Iterator i)
            {
                return i + n;
            }

            Iterator &operator-=(difference_type n)
            {
                cur = -= n * step;
                return *this;
            }

            friend Iterator operator-(Iterator i, difference_type n)
            {
                return i -= n;
            }

            friend difference_type operator-(const Iterator &b,
                                             const Iterator &a)
            {
                AGZ_ASSERT(b.step == a.step);
                return (b.cur - a.cur) / b.step;
            }

            T operator[](difference_type n) const
            {
                return cur + n * step;
            }

            friend bool operator==(const Iterator &a, const Iterator &b)
            {
                AGZ_ASSERT(a.step == b.step);
                return a.cur == b.cur;
            }

            friend bool operator!=(const Iterator &a, const Iterator &b)
            {
                return !(a == b);
            }

            friend bool operator<(const Iterator &a, const Iterator &b)
            {
                return (a.step < 0) ^ (a.cur < b.cur);
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

        BetweenImpl(T start, T step, T end)
            : start_(start), step_(step)
        {
            // IMPROVE
            if(step > 0)
            {
                AGZ_ASSERT(start <= end);
                while((end - start) % step)
                    ++end;
            }
            else
            {
                AGZ_ASSERT(start >= end);
                while((end - start) % step)
                    --end;
            }
            end_ = end;
        }

        Iterator begin() const
        {
            return Iterator(start_, step_);
        }

        Iterator end() const
        {
            return Iterator(end_, step_);
        }
    };
}

template<typename T>
RangeAux::BetweenImpl<T> Between(T start, T end)
{
    if(start <= end)
        return RangeAux::BetweenImpl<T>(start, end, 1);
    return RangeAux::BetweenImpl<T>(start, end, -1);
}

template<typename T>
RangeAux::BetweenImpl<T> Between(T start, T end, T step)
{
    return RangeAux::BetweenImpl<T>(start, end, step);
}

AGZ_NS_END(AGZ)
