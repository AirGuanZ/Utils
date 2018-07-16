#pragma once

#include <cstring>
#include <type_traits>

#include "../Common.h"
#include "Scalar.h"
#include "Vec2.h"
#include "Vec3.h"

AGZ_NS_BEG(AGZ::Math)

template<typename T>
class Vec4
{
public:

    union
    {
        struct { T u; T v; T m; T n; };
        struct { T x; T y; T z; T w; };
        struct { T r; T g; T b; T a; };
    };

public:

    using Data = T[4];
    using Component = T;
    using Self = Vec4<T>;

    AGZ_FORCE_INLINE Vec4() : Vec4(T(0)) { }

    explicit AGZ_FORCE_INLINE Vec4(Uninitialized_t) { }

    explicit AGZ_FORCE_INLINE Vec4(T value) : x(v), y(v), z(v), w(v) { }

    explicit AGZ_FORCE_INLINE Vec4(const T *data)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        std::memcpy(&x, data, sizeof(Data));
    }

    AGZ_FORCE_INLINE Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) { }

    AGZ_FORCE_INLINE Vec4(const Self &other) : x(other.x), y(other.y), z(other.z), w(other.w) { }

    AGZ_FORCE_INLINE Self &operator=(const Self &other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
        return *this;
    }

    template<typename U, typename F>
    Vec4<U> Map(F &&f) const { return Vec4<U>(f(x), f(y), f(z), f(w)); }

#include "SwizzleVec4.inl"

#define x r
#define y g
#define z b
#define w a

#include "SwizzleVec4.inl"

#undef x
#undef y
#undef z
#undef w

#define x u
#define y v
#define z m
#define w n

#include "SwizzleVec4.inl"

#undef x
#undef y
#undef z
#undef w

    AGZ_FORCE_INLINE       T &operator[](size_t idx) { AGZ_ASSERT(idx < 4); return (&x)[idx]; }
    AGZ_FORCE_INLINE const T &operator[](size_t idx) const { AGZ_ASSERT(idx < 4); return (&x)[idx]; }

    AGZ_FORCE_INLINE Self operator+(const Self &rhs) const { return Self(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
    AGZ_FORCE_INLINE Self operator-(const Self &rhs) const { return Self(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
    AGZ_FORCE_INLINE Self operator*(const Self &rhs) const { return Self(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
    AGZ_FORCE_INLINE Self operator/(const Self &rhs) const { return Self(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

    AGZ_FORCE_INLINE Self &operator+=(const Self &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
    AGZ_FORCE_INLINE Self &operator-=(const Self &rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
    AGZ_FORCE_INLINE Self &operator*=(const Self &rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
    AGZ_FORCE_INLINE Self &operator/=(const Self &rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }

    AGZ_FORCE_INLINE bool operator==(const Self &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
    AGZ_FORCE_INLINE bool operator!=(const Self &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w; }

    template<typename U>
    AGZ_FORCE_INLINE Self &operator+=(const U &rhs) { x += rhs; y += rhs; z += rhs; w += rhs; return *this; }
    template<typename U>
    AGZ_FORCE_INLINE Self &operator-=(const U &rhs) { x -= rhs; y -= rhs; z -= rhs; w -= rhs; return *this; }
    template<typename U>
    AGZ_FORCE_INLINE Self &operator*=(const U &rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
    template<typename U>
    AGZ_FORCE_INLINE Self &operator/=(const U &rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }

    static const Self &UNIT_X()
    {
        static const Self ret(T(1), T(0), T(0), T(0));
        return ret;
    }

    static const Self &UNIT_Y()
    {
        static const Self ret(T(0), T(1), T(0), T(0));
        return ret;
    }

    static const Self &UNIT_Z()
    {
        static const Self ret(T(0), T(0), T(1), T(0));
        return ret;
    }

    static const Self &UNIT_W()
    {
        static const Self ret(T(0), T(0), T(0), T(1));
        return ret;
    }
};

template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec4<T2> operator+(const T1 &lhs, const Vec4<T2> &rhs) { return Vec4<T2>(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec4<T1> operator+(const Vec4<T1> &lhs, const T2 &rhs) { return Vec4<T1>(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec4<T1> operator-(const Vec4<T1> &lhs, const T2 &rhs) { return Vec4<T1>(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec4<T2> operator*(const T1 &lhs, const Vec4<T2> &rhs) { return Vec4<T2>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec4<T1> operator*(const Vec4<T1> &lhs, const T2 &rhs) { return Vec4<T1>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec4<T1> operator/(const Vec4<T1> &lhs, const T2 &rhs) { return Vec4<T1>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs); }

template<typename T1, typename T2>
AGZ_FORCE_INLINE auto Dot(const Vec4<T1> &lhs, const Vec4<T2> &rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

template<typename T>
AGZ_FORCE_INLINE auto LengthSquare(const Vec4<T> &vec)
{
    return Dot(vec, vec);
}

template<typename T>
AGZ_FORCE_INLINE auto Length(const Vec4<T> &vec)
{
    return Sqrt(LengthSquare(vec));
}

template<typename T>
AGZ_FORCE_INLINE auto Normalize(const Vec4<T> &vec)
{
    return vec / Length(vec);
}

template<typename T>
AGZ_FORCE_INLINE auto Clamp(const Vec4<T> &vec, T minv, T maxv)
{
    return Vec4<decltype(Clamp(vec.x, minv, maxv))>(
                         Clamp(vec.x, minv, maxv),
                         Clamp(vec.y, minv, maxv),
                         Clamp(vec.z, minv, maxv),
                         Clamp(vec.w, minv, maxv));
}

template<typename T>
AGZ_FORCE_INLINE bool ApproxEq(const Vec4<T> &lhs, const Vec4<T> &rhs, T epsilon)
{
    return ApproxEq(lhs.x, rhs.x, epsilon) &&
           ApproxEq(lhs.y, rhs.y, epsilon) &&
           ApproxEq(lhs.z, rhs.z, epsilon) &&
           ApproxEq(lhs.w, rhs.w, epsilon);
}

template<typename T>
AGZ_FORCE_INLINE Vec4<T> Homogenize(const Vec4<T> &vec)
{
    T inv_w = T(1) / vec.w;
    return Vec4<T>(vec.x * inv_w, vec.y * inv_w, vec.z * inv_w, T(1));
}

using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;

AGZ_NS_END(AGZ::Math)
