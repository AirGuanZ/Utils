#pragma once

#include <limits>
#include <type_traits>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
class Seq
{
    T start_, step_;

public:

    class Iterator
    {
        T cur, step;

        Iterator(T cur, T step)
            : cur(cur), step(step)
        {

        }

        static constexpr T EndVal()
        {
            return std::numeric_limits<T>::lowest();
        }

        bool IsEnd() const
        {
            return step == EndVal();
        }

    public:

        friend class Seq;

        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::make_signed_t<T>;
        using pointer = T*;
        using reference = T&;

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
            Iterator ret = *this;
            cur += step;
            return ret;
        }

        Iterator &operator--()
        {
            cur -= step;
            return *this;
        }

        Iterator operator--(int)
        {
            Iterator ret = *this;
            cur -= step;
            return ret;
        }

        Iterator &operator+=(difference_type n)
        {
            cur += n * step;
            return *this;
        }

        friend Iterator operator+(const Iterator &i, difference_type n)
        {
            return Iterator(i.cur + n * i.step, i.step);
        }

        friend Iterator operator+(difference_type n, const Iterator &i)
        {
            return i + n;
        }

        Iterator &operator-=(difference_type n)
        {
            cur -= n * step;
            return *this;
        }

        friend Iterator operator-(const Iterator &i, difference_type n)
        {
            return Iterator(i.cur - n * i.step, i.step);
        }

        friend difference_type operator-(const Iterator &b, const Iterator &a)
        {
            if(b.IsEnd())
            {
                if(a.IsEnd()) return difference_type(0);
                return std::numeric_limits<difference_type>::max();
            }
            if(a.IsEnd())
                return std::numeric_limits<difference_type>::lowest();
            AGZ_ASSERT(b.step == a.step);
            return (b.cur - a.cur) / b.step;
        }

        T operator[](difference_type n) const
        {
            return cur + n * step;
        }

        friend bool operator==(const Iterator &a, const Iterator &b)
        {
            if(a.IsEnd() || b.IsEnd())
                return false;
            AGZ_ASSERT(a.step == b.step);
            return a.cur == b.cur;
        }

        friend bool operator!=(const Iterator &a, const Iterator &b)
        {
            return !(a == b);
        }

        friend bool operator<(const Iterator &a, const Iterator &b)
        {
            if(a.IsEnd())
                return false;
            if(b.IsEnd())
                return true;
            AGZ_ASSERT(a.step == b.step);
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

    explicit Seq(T start, T step = T(1))
        : start_(start), step_(step)
    {
        AGZ_ASSERT(step != 0);
    }

    Iterator begin() const
    {
        return Iterator(start_, step_);
    }

    Iterator end() const
    {
        return Iterator(T(0), Iterator::EndVal());
    }
};

AGZ_NS_END(AGZ)
