#pragma once

#include <cstring>
#include <type_traits>

#include "../Misc/Common.h"
#include "../Utils/Serialize.h"
#include "Scalar.h"
#include "Vec2.h"
#include "Vec3.h"

namespace AGZ::Math {

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

    using Data = T[4];
    using Component = T;
    using Self = Vec4<T>;

    constexpr Vec4() : Vec4(T(0)) { }

    explicit Vec4(Uninitialized_t) { }

    explicit constexpr Vec4(T value) : x(value), y(value), z(value), w(value) { }

    constexpr Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) { }

    explicit Vec4(const T *data)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        std::memcpy(&x, data, sizeof(Data));
    }

    Vec4(const Self &other) : x(other.x), y(other.y), z(other.z), w(other.w) { }

    Vec4(T param0, T param1, const Vec2<T> &param2)
        : x(param0), y(param1), z(param2.x), w(param2.y)
    {

    }

    Vec4(T param0, const Vec2<T> &param1, T param2)
        : x(param0), y(param1.x), z(param1.y), w(param2)
    {

    }

    Vec4(T param0, const Vec3<T> &param1)
        : x(param0), y(param1.x), z(param1.y), w(param1.z)
    {

    }

    Vec4(const Vec2<T> &param0, T param1, T param2)
        : x(param0.x), y(param0.y), z(param1), w(param2)
    {

    }

    Vec4(const Vec2<T> &param0, const Vec2<T> &param1)
        : x(param0.x), y(param0.y), z(param1.x), w(param1.y)
    {

    }

    Vec4(const Vec3<T> &param0, T param1)
        : x(param0.x), y(param0.y), z(param0.z), w(param1)
    {

    }

    Self &operator=(const Self &other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
        return *this;
    }

    template<typename F>
    auto Map(F &&f) const
    {
        using U = remove_rcv_t<decltype(f(x))>;
        return Vec4<U>(f(x), f(y), f(z), f(w));
    }

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

          T &operator[](size_t idx) { AGZ_ASSERT(idx < 4); return (&x)[idx]; }
    const T &operator[](size_t idx) const { AGZ_ASSERT(idx < 4); return (&x)[idx]; }

    Self operator+(const Self &rhs) const { return Self(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
    Self operator-(const Self &rhs) const { return Self(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
    Self operator*(const Self &rhs) const { return Self(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
    Self operator/(const Self &rhs) const { return Self(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

    Self &operator+=(const Self &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
    Self &operator-=(const Self &rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
    Self &operator*=(const Self &rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
    Self &operator/=(const Self &rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }

    bool operator==(const Self &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
    bool operator!=(const Self &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w; }

    template<typename U>
    Self &operator+=(const U &rhs) { x += rhs; y += rhs; z += rhs; w += rhs; return *this; }
    template<typename U>
    Self &operator-=(const U &rhs) { x -= rhs; y -= rhs; z -= rhs; w -= rhs; return *this; }
    template<typename U>
    Self &operator*=(const U &rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
    template<typename U>
    Self &operator/=(const U &rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }

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

    auto LengthSquare()      const;
    auto Length()            const;
    Self Normalize()         const;
    Self Clamp(T min, T max) const;
    Self Sqrt()              const;

    bool Serialize(BinarySerializer &serializer) const;
    bool Deserialize(BinaryDeserializer &deserializer);
};

template<typename T>
Vec4<T> operator-(const Vec4<T> &v) { return Vec4<T>(-v.x, -v.y, -v.z, -v.w); }

template<typename T1, typename T2>
Vec4<T2> operator+(const T1 &lhs, const Vec4<T2> &rhs) { return Vec4<T2>(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w); }
template<typename T1, typename T2>
Vec4<T1> operator+(const Vec4<T1> &lhs, const T2 &rhs) { return Vec4<T1>(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs); }
template<typename T1, typename T2>
Vec4<T1> operator-(const Vec4<T1> &lhs, const T2 &rhs) { return Vec4<T1>(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs); }
template<typename T1, typename T2, std::enable_if_t<std::is_arithmetic_v<T1>>>
Vec4<T2> operator*(const T1 &lhs, const Vec4<T2> &rhs) { return Vec4<T2>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w); }
template<typename T1, typename T2>
Vec4<T1> operator*(const Vec4<T1> &lhs, const T2 &rhs) { return Vec4<T1>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }
template<typename T1, typename T2>
Vec4<T1> operator/(const Vec4<T1> &lhs, const T2 &rhs) { return Vec4<T1>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs); }

template<typename T1, typename T2>
auto Dot(const Vec4<T1> &lhs, const Vec4<T2> &rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

template<typename T>
auto LengthSquare(const Vec4<T> &vec)
{
    return Dot(vec, vec);
}

template<typename T>
auto Length(const Vec4<T> &vec)
{
    return Sqrt(LengthSquare(vec));
}

template<typename T>
auto Normalize(const Vec4<T> &vec)
{
    return vec / Length(vec);
}

template<typename T>
auto Clamp(const Vec4<T> &vec, T minv, T maxv)
{
    return Vec4<decltype(Clamp(vec.x, minv, maxv))>(
                         Clamp(vec.x, minv, maxv),
                         Clamp(vec.y, minv, maxv),
                         Clamp(vec.z, minv, maxv),
                         Clamp(vec.w, minv, maxv));
}

template<typename T>
auto Sqrt(const Vec4<T> &v)
{
    return Vec4<decltype(Sqrt(v.x))>(Sqrt(v.x), Sqrt(v.y), Sqrt(v.y), Sqrt(v.w));
}

template<typename T>
bool ApproxEq(const Vec4<T> &lhs, const Vec4<T> &rhs, T epsilon)
{
    return ApproxEq(lhs.x, rhs.x, epsilon) &&
           ApproxEq(lhs.y, rhs.y, epsilon) &&
           ApproxEq(lhs.z, rhs.z, epsilon) &&
           ApproxEq(lhs.w, rhs.w, epsilon);
}

template<typename T>
Vec4<T> Homogenize(const Vec4<T> &vec)
{
    T inv_w = T(1) / vec.w;
    return Vec4<T>(vec.x * inv_w, vec.y * inv_w, vec.z * inv_w, T(1));
}

template<typename T>
auto Brightness(const Vec4<T> &v)
{
    return T(0.2126) * v.r + T(0.7152) * v.g + T(0.0722) * v.b;
}

template<typename T>
auto Vec4<T>::LengthSquare() const
{
    return ::AGZ::Math::LengthSquare(*this);
}

template<typename T>
auto Vec4<T>::Length() const
{
    return ::AGZ::Math::Length(*this);
}

template<typename T>
Vec4<T> Vec4<T>::Normalize() const
{
    return ::AGZ::Math::Normalize(*this);
}

template<typename T>
Vec4<T> Vec4<T>::Clamp(T min, T max) const
{
    return ::AGZ::Math::Clamp(*this, min, max);
}

template<typename T>
Vec4<T> Vec4<T>::Sqrt() const
{
    return ::AGZ::Math::Sqrt(*this);
}

template<typename T>
bool Vec4<T>::Serialize(BinarySerializer &serializer) const
{
    return serializer.Serialize(x) &&
           serializer.Serialize(y) &&
           serializer.Serialize(z) &&
           serializer.Serialize(w);
}

template<typename T>
bool Vec4<T>::Deserialize(BinaryDeserializer &deserializer)
{
    return deserializer.Deserialize(x) &&
           deserializer.Deserialize(y) &&
           deserializer.Deserialize(z) &&
           deserializer.Deserialize(w);
}

using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;

} // namespace AGZ::Math
