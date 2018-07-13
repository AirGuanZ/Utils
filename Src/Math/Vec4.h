#pragma once

#include "../Common.h"
#include "Scalar.h"

namespace Math
{
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

        using Component = T;
        using Self = Vec4<T>;

        AGZ_FORCE_INLINE Vec4()
            : x(Scalar::ZERO<T>()), y(Scalar::ZERO<T>()), z(Scalar::ZERO<T>()), w(Scalar::ZERO<T>())
        {

        }

        explicit AGZ_FORCE_INLINE Vec4(const T &value)
            : x(v), y(v), z(v), w(v)
        {

        }

        AGZ_FORCE_INLINE Vec4(const T &x, const T &y, const T &z, const T &w)
            : x(x), y(y), z(z), w(w)
        {

        }

        AGZ_FORCE_INLINE Vec4(const Self &other)
            : x(other.x), y(other.y), z(other.z), w(other.w)
        {

        }

        AGZ_FORCE_INLINE Self &operator=(const Self &other)
        {
            x = other.x;
            y = other.y;
            z = other.z;
            w = other.w;
            return *this;
        }

        AGZ_FORCE_INLINE       T &operator[](size_t idx)       { AGZ_ASSERT(idx < 4); return (&x)[idx]; }
        AGZ_FORCE_INLINE const T &operator[](size_t idx) const { AGZ_ASSERT(idx < 4); return (&x)[idx]; }

        AGZ_FORCE_INLINE Self operator+(const Self &rhs) const { return Self(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
        AGZ_FORCE_INLINE Self operator-(const Self &rhs) const { return Self(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
        AGZ_FORCE_INLINE Self operator*(const Self &rhs) const { return Self(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
        AGZ_FORCE_INLINE Self operator/(const Self &rhs) const { return Self(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

        AGZ_FORCE_INLINE Self &operator+=(const Self &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
        AGZ_FORCE_INLINE Self &operator-=(const Self &rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
        AGZ_FORCE_INLINE Self &operator*=(const Self &rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
        AGZ_FORCE_INLINE Self &operator/=(const Self &rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }

        AGZ_FORCE_INLINE bool operator==(const Self &rhs) { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
        AGZ_FORCE_INLINE bool operator!=(const Self &rhs) { return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w; }

        template<typename U>
        AGZ_FORCE_INLINE Self &operator+=(const U &rhs) { x += rhs; y += rhs; z += rhs; w += rhs; return *this; }
        template<typename U>
        AGZ_FORCE_INLINE Self &operator-=(const U &rhs) { x -= rhs; y -= rhs; z -= rhs; w -= rhs; return *this; }
        template<typename U>
        AGZ_FORCE_INLINE Self &operator*=(const U &rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
        template<typename U>
        AGZ_FORCE_INLINE Self &operator/=(const U &rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }

        static const Self &ZERO()
        {
            static const Self ret(Scalar::ZERO<T>(), Scalar::ZERO<T>(), Scalar::ZERO<T>(), Scalar::ZERO<T>());
            return ret;
        }

        static const Self &ONES()
        {
            static const Self ret(Scalar::ONE<T>(), Scalar::ONE<T>(), Scalar::ONE<T>(), Scalar::ONE<T>());
            return ret;
        }

        static const Self &UNIT_X()
        {
            static const Self ret(Scalar::ONE<T>(), Scalar::ZERO<T>(), Scalar::ZERO<T>(), Scalar::ZERO<T>());
            return ret;
        }

        static const Self &UNIT_Y()
        {
            static const Self ret(Scalar::ZERO<T>(), Scalar::ONE<T>(), Scalar::ZERO<T>(), Scalar::ZERO<T>());
            return ret;
        }

        static const Self &UNIT_Z()
        {
            static const Self ret(Scalar::ZERO<T>(), Scalar::ZERO<T>(), Scalar::ONE<T>(), Scalar::ZERO<T>());
            return ret;
        }

        static const Self &UNIT_W()
        {
            static const Self ret(Scalar::ZERO<T>(), Scalar::ZERO<T>(), Scalar::ZERO<T>(), Scalar::ONE<T>());
            return ret;
        }
    };

    template<typename T1, typename T2> AGZ_FORCE_INLINE
        auto operator+(const T1 &lhs, const Vec4<T2> &rhs) { return Vec4<decltype(lhs + rhs.x)>(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w); }
    template<typename T1, typename T2> AGZ_FORCE_INLINE
        auto operator+(const Vec4<T1> &lhs, const T2 &rhs) { return Vec4<decltype(lhs.x + rhs)>(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs); }
    template<typename T1, typename T2> AGZ_FORCE_INLINE
        auto operator-(const Vec4<T1> &lhs, const T2 &rhs) { return Vec4<decltype(lhs.x - rhs)>(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs); }
    template<typename T1, typename T2> AGZ_FORCE_INLINE
        auto operator*(const T1 &lhs, const Vec4<T2> &rhs) { return Vec4<decltype(lhs * rhs.x)>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w); }
    template<typename T1, typename T2> AGZ_FORCE_INLINE
        auto operator*(const Vec4<T1> &lhs, const T2 &rhs) { return Vec4<decltype(lhs.x * rhs)>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }
    template<typename T1, typename T2> AGZ_FORCE_INLINE
        auto operator/(const Vec4<T1> &lhs, const T2 &rhs) { return Vec4<decltype(lhs.x / rhs)>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs); }

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
        return Scalar::Sqrt(LengthSquare(vec));
    }

    template<typename T>
    AGZ_FORCE_INLINE auto Normalize(const Vec4<T> &vec)
    {
        return vec / Length(v);
    }

    template<typename T>
    AGZ_FORCE_INLINE auto Clamp(const Vec4<T> &vec, T minv, T maxv)
    {
        return Vec4<decltype(Scalar::Clamp(vec.x, minv, maxv))>(
            Scalar::Clamp(vec.x, minv, maxv),
            Scalar::Clamp(vec.y, minv, maxv),
            Scalar::Clamp(vec.z, minv, maxv),
            Scalar::Clamp(vec.w, minv, maxv));
    }
}
