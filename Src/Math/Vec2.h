#pragma once

#include <cstring>
#include <type_traits>

#include "../Common.h"
#include "Scalar.h"

AGZ_NS_BEG(AGZ::Math)

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

    using Data = T[2];
    using Component = T;
    using Self = Vec2<T>;

    AGZ_FORCE_INLINE Vec2() : Vec2(T(0)) { }

    explicit AGZ_FORCE_INLINE Vec2(Uninitialized_t) { }

    explicit AGZ_FORCE_INLINE Vec2(T value) : x(value), y(value) { }

    AGZ_FORCE_INLINE Vec2(T x, T y) : x(x), y(y) { }

    AGZ_FORCE_INLINE Vec2(const Self &other) : x(other.x), y(other.y) { }

    explicit AGZ_FORCE_INLINE Vec2(const T *data)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        std::memcpy(&x, data, sizeof(Data));
    }

    AGZ_FORCE_INLINE Self &operator=(const Self &other)
    {
        x = other.x;
        y = other.y;
        return *this;
    }

    template<typename U, typename F>
    Vec2<U> Map(F &&f) const { return Vec2<U>(f(x), f(y)); }

#include "SwizzleVec2.inl"

#define x u
#define y v

#include "SwizzleVec2.inl"

#undef x
#undef y

    AGZ_FORCE_INLINE       T &operator[](size_t idx) { AGZ_ASSERT(idx < 2); return (&x)[idx]; }
    AGZ_FORCE_INLINE const T &operator[](size_t idx) const { AGZ_ASSERT(idx < 2); return (&x)[idx]; }

    AGZ_FORCE_INLINE Self operator+(const Self &rhs) const { return Self(x + rhs.x, y + rhs.y); }
    AGZ_FORCE_INLINE Self operator-(const Self &rhs) const { return Self(x - rhs.x, y - rhs.y); }
    AGZ_FORCE_INLINE Self operator*(const Self &rhs) const { return Self(x * rhs.x, y * rhs.y); }
    AGZ_FORCE_INLINE Self operator/(const Self &rhs) const { return Self(x / rhs.x, y / rhs.y); }

    AGZ_FORCE_INLINE Self &operator+=(const Self &rhs) { x += rhs.x; y += rhs.y; return *this; }
    AGZ_FORCE_INLINE Self &operator-=(const Self &rhs) { x -= rhs.x; y -= rhs.y; return *this; }
    AGZ_FORCE_INLINE Self &operator*=(const Self &rhs) { x *= rhs.x; y *= rhs.y; return *this; }
    AGZ_FORCE_INLINE Self &operator/=(const Self &rhs) { x /= rhs.x; y /= rhs.y; return *this; }

    AGZ_FORCE_INLINE bool operator==(const Self &rhs) const { return x == rhs.x && y == rhs.y; }
    AGZ_FORCE_INLINE bool operator!=(const Self &rhs) const { return x != rhs.x || y != rhs.y; }

    template<typename U>
    AGZ_FORCE_INLINE Self &operator+=(const U &rhs) { x += rhs; y += rhs; return *this; }
    template<typename U>
    AGZ_FORCE_INLINE Self &operator-=(const U &rhs) { x -= rhs; y -= rhs; return *this; }
    template<typename U>
    AGZ_FORCE_INLINE Self &operator*=(const U &rhs) { x *= rhs; y *= rhs; return *this; }
    template<typename U>
    AGZ_FORCE_INLINE Self &operator/=(const U &rhs) { x /= rhs; y /= rhs; return *this; }

    static const Self &UNIT_X()
    {
        static const Self ret(T(1), T(0));
        return ret;
    }

    static const Self &UNIT_Y()
    {
        static const Self ret(T(0), T(1));
        return ret;
    }
};

template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec2<T2> operator+(const T1 &lhs, const Vec2<T2> &rhs) { return Vec2<T2>(lhs + rhs.x, lhs + rhs.y); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec2<T1> operator+(const Vec2<T1> &lhs, const T2 &rhs) { return Vec2<T1>(lhs.x + rhs, lhs.y + rhs); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec2<T1> operator-(const Vec2<T1> &lhs, const T2 &rhs) { return Vec2<T1>(lhs.x - rhs, lhs.y - rhs); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec2<T2> operator*(const T1 &lhs, const Vec2<T2> &rhs) { return Vec2<T2>(lhs * rhs.x, lhs * rhs.y); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec2<T1> operator*(const Vec2<T1> &lhs, const T2 &rhs) { return Vec2<T1>(lhs.x * rhs, lhs.y * rhs); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec2<T1> operator/(const Vec2<T1> &lhs, const T2 &rhs) { return Vec2<T1>(lhs.x / rhs, lhs.y / rhs); }

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
    return Sqrt(LengthSquare(vec));
}

template<typename T>
AGZ_FORCE_INLINE auto Normalize(const Vec2<T> &vec)
{
    return vec / Length(vec);
}

template<typename T>
AGZ_FORCE_INLINE auto Clamp(const Vec2<T> &vec, T minv, T maxv)
{
    return Vec2<decltype(Clamp(vec.x, minv, maxv))>(
        Clamp(vec.x, minv, maxv), Clamp(vec.y, minv, maxv));
}

template<typename T>
AGZ_FORCE_INLINE auto Sqrt(const Vec2<T> &v)
{
    return Vec2<decltype(Sqrt(v.x))>(Sqrt(v.x), Sqrt(v.y));
}

template<typename T>
AGZ_FORCE_INLINE bool ApproxEq(const Vec2<T> &lhs, const Vec2<T> &rhs, T epsilon)
{
    return ApproxEq(lhs.x, rhs.x, epsilon) &&
           ApproxEq(lhs.y, rhs.y, epsilon);
}

using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;

AGZ_NS_END(AGZ::Math)
