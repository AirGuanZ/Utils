#pragma once

#include <type_traits>

#include "../Misc/Common.h"

namespace AGZ {

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

            const T *operator->() const
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
                cur -= step;
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
                cur += n * step;
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
                cur -= n * step;
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

        BetweenImpl(T start, T end, T step)
            : start_(start), step_(step)
        {
            if(step > 0)
            {
                AGZ_ASSERT(start <= end);
                end += (end - start) % step;
            }
            else
            {
                // IMPROVE
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

/**
 * @brief 以1/-1为步长，某个区间中所有的整数
 * 
 * @param start 第一个整数
 * @param end 区间的另一个边界，不包含在range内
 */
template<typename T>
RangeAux::BetweenImpl<T> Between(T start, T end)
{
    if(start <= end)
        return RangeAux::BetweenImpl<T>(start, end, 1);
    return RangeAux::BetweenImpl<T>(start, end, -1);
}

/**
 * @brief 以指定步长扫描某个整数区间得到的range
 * 
 * @param start 第一个整数
 * @param end 区间的另一个边界，不包含在range内
 * @param step 扫描步长
 */
template<typename T>
RangeAux::BetweenImpl<T> Between(T start, T end, T step)
{
    return RangeAux::BetweenImpl<T>(start, end, step);
}

} // namespace AGZ
