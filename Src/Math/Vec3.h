#pragma once

#include "../Common.h"
#include "Scalar.h"

AGZ_NS_BEG(AGZ)
AGZ_NS_BEG(Math)

template<typename T>
class Vec3
{
public:

    union
    {
        struct { T u; T v; T m; };
        struct { T x; T y; T z; };
        struct { T r; T g; T b; };
    };

public:

    using Component = T;
    using Self = Vec3<T>;

    AGZ_FORCE_INLINE Vec3()
        : x(Scalar::ZERO<T>()), y(Scalar::ZERO<T>()), z(Scalar::ZERO<T>())
    {

    }

    explicit AGZ_FORCE_INLINE Vec3(const T &value)
        : x(v), y(v), z(v)
    {

    }

    AGZ_FORCE_INLINE Vec3(const T &x, const T &y, const T &z)
        : x(x), y(y), z(z)
    {

    }

    AGZ_FORCE_INLINE Vec3(const Self &other)
        : x(other.x), y(other.y), z(other.z)
    {

    }

    AGZ_FORCE_INLINE Self &operator=(const Self &other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    AGZ_FORCE_INLINE       T &operator[](size_t idx) { AGZ_ASSERT(idx < 3); return (&x)[idx]; }
    AGZ_FORCE_INLINE const T &operator[](size_t idx) const { AGZ_ASSERT(idx < 3); return (&x)[idx]; }

    AGZ_FORCE_INLINE Self operator+(const Self &rhs) const { return Self(x + rhs.x, y + rhs.y, z + rhs.z); }
    AGZ_FORCE_INLINE Self operator-(const Self &rhs) const { return Self(x - rhs.x, y - rhs.y, z - rhs.z); }
    AGZ_FORCE_INLINE Self operator*(const Self &rhs) const { return Self(x * rhs.x, y * rhs.y, z * rhs.z); }
    AGZ_FORCE_INLINE Self operator/(const Self &rhs) const { return Self(x / rhs.x, y / rhs.y, z / rhs.z); }

    AGZ_FORCE_INLINE Self &operator+=(const Self &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
    AGZ_FORCE_INLINE Self &operator-=(const Self &rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
    AGZ_FORCE_INLINE Self &operator*=(const Self &rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
    AGZ_FORCE_INLINE Self &operator/=(const Self &rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

    AGZ_FORCE_INLINE bool operator==(const Self &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
    AGZ_FORCE_INLINE bool operator!=(const Self &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }

    template<typename U>
    AGZ_FORCE_INLINE Self &operator+=(const U &rhs) { x += rhs; y += rhs; z += rhs; return *this; }
    template<typename U>
    AGZ_FORCE_INLINE Self &operator-=(const U &rhs) { x -= rhs; y -= rhs; z -= rhs; return *this; }
    template<typename U>
    AGZ_FORCE_INLINE Self &operator*=(const U &rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
    template<typename U>
    AGZ_FORCE_INLINE Self &operator/=(const U &rhs) { x /= rhs; y /= rhs; z /= rhs; return *this; }

    static const Self &ZERO()
    {
        static const Self ret(Scalar::ZERO<T>(), Scalar::ZERO<T>(), Scalar::ZERO<T>());
        return ret;
    }

    static const Self &ONES()
    {
        static const Self ret(Scalar::ONE<T>(), Scalar::ONE<T>(), Scalar::ONE<T>());
        return ret;
    }

    static const Self &UNIT_X()
    {
        static const Self ret(Scalar::ONE<T>(), Scalar::ZERO<T>(), Scalar::ZERO<T>());
        return ret;
    }

    static const Self &UNIT_Y()
    {
        static const Self ret(Scalar::ZERO<T>(), Scalar::ONE<T>(), Scalar::ZERO<T>());
        return ret;
    }

    static const Self &UNIT_Z()
    {
        static const Self ret(Scalar::ZERO<T>(), Scalar::ZERO<T>(), Scalar::ONE<T>());
        return ret;
    }
};

template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec3<T2> operator+(const T1 &lhs, const Vec3<T2> &rhs) { return Vec3<T2>(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec3<T1> operator+(const Vec3<T1> &lhs, const T2 &rhs) { return Vec3<T1>(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec3<T1> operator-(const Vec3<T1> &lhs, const T2 &rhs) { return Vec3<T1>(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec3<T2> operator*(const T1 &lhs, const Vec3<T2> &rhs) { return Vec3<T2>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec3<T1> operator*(const Vec3<T1> &lhs, const T2 &rhs) { return Vec3<T1>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs); }
template<typename T1, typename T2> AGZ_FORCE_INLINE
Vec3<T1> operator/(const Vec3<T1> &lhs, const T2 &rhs) { return Vec3<T1>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs); }

template<typename T1, typename T2>
AGZ_FORCE_INLINE auto Dot(const Vec3<T1> &lhs, const Vec3<T2> &rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template<typename T>
AGZ_FORCE_INLINE auto LengthSquare(const Vec3<T> &vec)
{
    return Dot(vec, vec);
}

template<typename T>
AGZ_FORCE_INLINE auto Length(const Vec3<T> &vec)
{
    return Scalar::Sqrt(LengthSquare(vec));
}

template<typename T>
AGZ_FORCE_INLINE auto Normalize(const Vec3<T> &vec)
{
    return vec / Length(v);
}

template<typename T>
AGZ_FORCE_INLINE auto Clamp(const Vec3<T> &vec, T minv, T maxv)
{
    return Vec3<decltype(Scalar::Clamp(vec.x, minv, maxv))>(
        Scalar::Clamp(vec.x, minv, maxv),
        Scalar::Clamp(vec.y, minv, maxv),
        Scalar::Clamp(vec.z, minv, maxv));
}

AGZ_NS_END(Math)
AGZ_NS_END(AGZ)
