#pragma once

#include "../Common.h"
#include "Scalar.h"

namespace Math
{
    template<typename T>
    class Vec2
    {
    public:

        union
        {
            struct { T u; T v; };
            struct { T x; T y; };
        };

    public:

        using Component = T;
        using Self = Vec2<T>;

        AGZ_FORCE_INLINE Vec2()
            : x(Scalar::ZERO<T>()), y(Scalar::ZERO<T>())
        {

        }

        explicit AGZ_FORCE_INLINE Vec2(const T &value)
            : x(v), y(v)
        {

        }

        AGZ_FORCE_INLINE Vec2(const T &x, const T &y)
            : x(x), y(y)
        {

        }

        AGZ_FORCE_INLINE Vec2(const Self &other)
            : x(other.x), y(other.y)
        {

        }

        AGZ_FORCE_INLINE Self &operator=(const Self &other)
        {
            x = other.x;
            y = other.y;
            return *this;
        }

        AGZ_FORCE_INLINE       T &operator[](size_t idx)       { AGZ_ASSERT(idx < 2); return (&x)[idx]; }
        AGZ_FORCE_INLINE const T &operator[](size_t idx) const { AGZ_ASSERT(idx < 2); return (&x)[idx]; }

        AGZ_FORCE_INLINE Self operator+(const Self &rhs) const { return Self(x + rhs.x, y + rhs.y); }
        AGZ_FORCE_INLINE Self operator-(const Self &rhs) const { return Self(x - rhs.x, y - rhs.y); }
        AGZ_FORCE_INLINE Self operator*(const Self &rhs) const { return Self(x * rhs.x, y * rhs.y); }
        AGZ_FORCE_INLINE Self operator/(const Self &rhs) const { return Self(x / rhs.x, y / rhs.y); }

        AGZ_FORCE_INLINE Self &operator+=(const Self &rhs) { x += rhs.x; y += rhs.y; return *this; }
        AGZ_FORCE_INLINE Self &operator-=(const Self &rhs) { x -= rhs.x; y -= rhs.y; return *this; }
        AGZ_FORCE_INLINE Self &operator*=(const Self &rhs) { x *= rhs.x; y *= rhs.y; return *this; }
        AGZ_FORCE_INLINE Self &operator/=(const Self &rhs) { x /= rhs.x; y /= rhs.y; return *this; }

        AGZ_FORCE_INLINE bool operator==(const Self &rhs) { return x == rhs.x && y == rhs.y; }
        AGZ_FORCE_INLINE bool operator!=(const Self &rhs) { return x != rhs.x || y != rhs.y; }

        template<typename U>
        AGZ_FORCE_INLINE Self &operator+=(const U &rhs) { x += rhs; y += rhs; return *this; }
        template<typename U>
        AGZ_FORCE_INLINE Self &operator-=(const U &rhs) { x -= rhs; y -= rhs; return *this; }
        template<typename U>
        AGZ_FORCE_INLINE Self &operator*=(const U &rhs) { x *= rhs; y *= rhs; return *this; }
        template<typename U>
        AGZ_FORCE_INLINE Self &operator/=(const U &rhs) { x /= rhs; y /= rhs; return *this; }

        static const Self &ZERO()
        {
            static const Self ret(Scalar::ZERO<T>(), Scalar::ZERO<T>());
            return ret;
        }

        static const Self &ONES()
        {
            static const Self ret(Scalar::ONE<T>(), Scalar::ONE<T>());
            return ret;
        }

        static const Self &UNIT_X()
        {
            static const Self ret(Scalar::ONE<T>(), Scalar::ZERO<T>());
            return ret;
        }

        static const Self &UNIT_Y()
        {
            static const Self ret(Scalar::ZERO<T>(), Scalar::ONE<T>());
            return ret;
        }
    };

    template<typename T1, typename T2> AGZ_FORCE_INLINE
        auto operator+(const T1 &lhs, const Vec2<T2> &rhs) { return Vec2<decltype(lhs + rhs.x)>(lhs + rhs.x, lhs + rhs.y); }
    template<typename T1, typename T2> AGZ_FORCE_INLINE
        auto operator+(const Vec2<T1> &lhs, const T2 &rhs) { return Vec2<decltype(lhs.x + rhs)>(lhs.x + rhs, lhs.y + rhs); }
    template<typename T1, typename T2> AGZ_FORCE_INLINE
        auto operator-(const Vec2<T1> &lhs, const T2 &rhs) { return Vec2<decltype(lhs.x - rhs)>(lhs.x - rhs, lhs.y - rhs); }
    template<typename T1, typename T2> AGZ_FORCE_INLINE
        auto operator*(const T1 &lhs, const Vec2<T2> &rhs) { return Vec2<decltype(lhs * rhs.x)>(lhs * rhs.x, lhs * rhs.y); }
    template<typename T1, typename T2> AGZ_FORCE_INLINE
        auto operator*(const Vec2<T1> &lhs, const T2 &rhs) { return Vec2<decltype(lhs.x * rhs)>(lhs.x * rhs, lhs.y * rhs); }
    template<typename T1, typename T2> AGZ_FORCE_INLINE
        auto operator/(const Vec2<T1> &lhs, const T2 &rhs) { return Vec2<decltype(lhs.x / rhs)>(lhs.x / rhs, lhs.y / rhs); }

    template<typename T1, typename T2>
    AGZ_FORCE_INLINE auto Dot(const Vec2<T1> &lhs, const Vec2<T2> &rhs)
    {
        return lhs.x * rhs.x + lhs.y * rhs.y;
    }

    template<typename T>
    AGZ_FORCE_INLINE auto LengthSquare(const Vec2<T> &vec)
    {
        return Dot(vec, vec);
    }

    template<typename T>
    AGZ_FORCE_INLINE auto Length(const Vec2<T> &vec)
    {
        return Scalar::Sqrt(LengthSquare(vec));
    }

    template<typename T>
    AGZ_FORCE_INLINE auto Normalize(const Vec2<T> &vec)
    {
        return vec / Length(v);
    }

    template<typename T>
    AGZ_FORCE_INLINE auto Clamp(const Vec2<T> &vec, T minv, T maxv)
    {
        return Vec2<decltype(Scalar::Clamp(vec.x, minv, maxv))>(
            Scalar::Clamp(vec.x, minv, maxv), Scalar::Clamp(vec.y, minv, maxv));
    }
}
