#pragma once

#include <cstring>
#include <type_traits>

#include "../Misc/Common.h"
#include "Scalar.h"
#include "Vec2.h"

AGZ_NS_BEG(AGZ::Math)

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

    using Data = T[3];
    using Component = T;
    using Self = Vec3<T>;

    constexpr Vec3() : Vec3(T(0)) { }

    explicit Vec3(Uninitialized_t) { }

    explicit constexpr Vec3(T value) : x(value), y(value), z(value) { }

    constexpr Vec3(T x, T y, T z) : x(x), y(y), z(z) { }

    Vec3(const Self &other) : x(other.x), y(other.y), z(other.z) { }

    explicit Vec3(const T *data)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        std::memcpy(&x, data, sizeof(Data));
    }

    Self &operator=(const Self &other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    template<typename U, typename F>
    Vec3<U> Map(F &&f) const { return Vec3<U>(f(x), f(y), f(z)); }

#include "SwizzleVec3.inl"

#define x r
#define y g
#define z b

#include "SwizzleVec3.inl"

#undef x
#undef y
#undef z

#define x u
#define y v
#define z m

#include "SwizzleVec3.inl"

#undef x
#undef y
#undef z

          T &operator[](size_t idx) { AGZ_ASSERT(idx < 3); return (&x)[idx]; }
    const T &operator[](size_t idx) const { AGZ_ASSERT(idx < 3); return (&x)[idx]; }

    Self operator+(const Self &rhs) const { return Self(x + rhs.x, y + rhs.y, z + rhs.z); }
    Self operator-(const Self &rhs) const { return Self(x - rhs.x, y - rhs.y, z - rhs.z); }
    Self operator*(const Self &rhs) const { return Self(x * rhs.x, y * rhs.y, z * rhs.z); }
    Self operator/(const Self &rhs) const { return Self(x / rhs.x, y / rhs.y, z / rhs.z); }

    Self &operator+=(const Self &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
    Self &operator-=(const Self &rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
    Self &operator*=(const Self &rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
    Self &operator/=(const Self &rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

    bool operator==(const Self &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
    bool operator!=(const Self &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }

    template<typename U>
    Self &operator+=(const U &rhs) { x += rhs; y += rhs; z += rhs; return *this; }
    template<typename U>
    Self &operator-=(const U &rhs) { x -= rhs; y -= rhs; z -= rhs; return *this; }
    template<typename U>
    Self &operator*=(const U &rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
    template<typename U>
    Self &operator/=(const U &rhs) { x /= rhs; y /= rhs; z /= rhs; return *this; }

    static const Self &UNIT_X()
    {
        static const Self ret(T(1), T(0), T(0));
        return ret;
    }

    static const Self &UNIT_Y()
    {
        static const Self ret(T(0), T(1), T(0));
        return ret;
    }

    static const Self &UNIT_Z()
    {
        static const Self ret(T(0), T(0), T(1));
        return ret;
    }

    auto LengthSquare()      const;
    auto Length()            const;
    Self Normalize()         const;
    Self Clamp(T min, T max) const;
    Self Sqrt()              const;
};

template<typename T>
Vec3<T> operator-(const Vec3<T> &v) { return Vec3<T>(-v.x, -v.y, -v.z); }

template<typename T1, typename T2>
Vec3<T2> operator+(const T1 &lhs, const Vec3<T2> &rhs) { return Vec3<T2>(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z); }
template<typename T1, typename T2>
Vec3<T1> operator+(const Vec3<T1> &lhs, const T2 &rhs) { return Vec3<T1>(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs); }
template<typename T1, typename T2>
Vec3<T1> operator-(const Vec3<T1> &lhs, const T2 &rhs) { return Vec3<T1>(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs); }
template<typename T1, typename T2>
Vec3<T2> operator*(const T1 &lhs, const Vec3<T2> &rhs) { return Vec3<T2>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z); }
template<typename T1, typename T2>
Vec3<T1> operator*(const Vec3<T1> &lhs, const T2 &rhs) { return Vec3<T1>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs); }
template<typename T1, typename T2>
Vec3<T1> operator/(const Vec3<T1> &lhs, const T2 &rhs) { return Vec3<T1>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs); }

template<typename T>
auto Cross(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
    return Vec3<T>(lhs.y * rhs.z - lhs.z * rhs.y,
                   lhs.z * rhs.x - lhs.x * rhs.z,
                   lhs.x * rhs.y - lhs.y * rhs.x);
}

template<typename T1, typename T2>
auto Dot(const Vec3<T1> &lhs, const Vec3<T2> &rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template<typename T>
auto LengthSquare(const Vec3<T> &vec)
{
    return Dot(vec, vec);
}

template<typename T>
auto Length(const Vec3<T> &vec)
{
    return Sqrt(LengthSquare(vec));
}

template<typename T>
auto Normalize(const Vec3<T> &vec)
{
    return vec / Length(vec);
}

template<typename T>
auto Clamp(const Vec3<T> &vec, T minv, T maxv)
{
    return Vec3<decltype(Clamp(vec.x, minv, maxv))>(
                    Clamp(vec.x, minv, maxv),
                    Clamp(vec.y, minv, maxv),
                    Clamp(vec.z, minv, maxv));
}

template<typename T>
auto Sqrt(const Vec3<T> &v)
{
    return Vec3<decltype(Sqrt(v.x))>(Sqrt(v.x), Sqrt(v.y), Sqrt(v.y));
}

template<typename T>
bool ApproxEq(const Vec3<T> &lhs, const Vec3<T> &rhs, T epsilon)
{
    return ApproxEq(lhs.x, rhs.x, epsilon) &&
           ApproxEq(lhs.y, rhs.y, epsilon) &&
           ApproxEq(lhs.z, rhs.z, epsilon);
}

template<typename T>
auto Brightness(const Vec3<T> &v)
{
    return T(0.2126) * v.r + T(0.7152) * v.g + T(0.0722) * v.b;
}

template<typename T>
auto Vec3<T>::LengthSquare() const
{
    return ::AGZ::Math::LengthSquare(*this);
}

template<typename T>
auto Vec3<T>::Length() const
{
    return ::AGZ::Math::Length(*this);
}

template<typename T>
Vec3<T> Vec3<T>::Normalize() const
{
    return ::AGZ::Math::Normalize(*this);
}

template<typename T>
Vec3<T> Vec3<T>::Clamp(T min, T max) const
{
    return ::AGZ::Math::Clamp(*this, min, max);
}

template<typename T>
Vec3<T> Vec3<T>::Sqrt() const
{
    return ::AGZ::Math::Sqrt(*this);
}

using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;

AGZ_NS_END(AGZ::Math)
