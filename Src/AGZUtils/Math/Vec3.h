#pragma once

#include <cstring>
#include <type_traits>

#include "../Misc/Common.h"
#include "../Utils/Serialize.h"
#include "../Utils/String.h"
#include "Scalar.h"
#include "Vec2.h"

namespace AGZ::Math {

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

    constexpr Vec3() noexcept: Vec3(T(0)) { }

    explicit Vec3(Uninitialized_t) noexcept { }

    explicit constexpr Vec3(T value) noexcept: x(value), y(value), z(value) { }

    constexpr Vec3(T x, T y, T z) noexcept: x(x), y(y), z(z) { }

    Vec3(const Self &other) noexcept: x(other.x), y(other.y), z(other.z) { }

    explicit Vec3(const T *data) noexcept
    {
        static_assert(std::is_trivially_copyable_v<T>);
        std::memcpy(&x, data, sizeof(Data));
    }

    Vec3(T param0, const Vec2<T> &param1) noexcept
        : x(param0), y(param1.x), z(param1.y)
    {

    }

    Vec3(const Vec2<T> &param0, T param1) noexcept
        : x(param0.x), y(param0.y), z(param1)
    {

    }

    Self &operator=(const Self &other) noexcept
    {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    template<typename F>
    auto Map(F &&f) const noexcept
    {
        using U = remove_rcv_t<decltype(f(x))>;
        return Vec3<U>(f(x), f(y), f(z));
    }

    bool HasInf() const noexcept
    {
        return FP<T>(x).IsInfinity() || FP<T>(y).IsInfinity() || FP<T>(z).IsInfinity();
    }

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

          T &operator[](size_t idx) noexcept { AGZ_ASSERT(idx < 3); return (&x)[idx]; }
    const T &operator[](size_t idx) const noexcept { AGZ_ASSERT(idx < 3); return (&x)[idx]; }

    Self operator+(const Self &rhs) const noexcept { return Self(x + rhs.x, y + rhs.y, z + rhs.z); }
    Self operator-(const Self &rhs) const noexcept { return Self(x - rhs.x, y - rhs.y, z - rhs.z); }
    Self operator*(const Self &rhs) const noexcept { return Self(x * rhs.x, y * rhs.y, z * rhs.z); }
    Self operator/(const Self &rhs) const noexcept { return Self(x / rhs.x, y / rhs.y, z / rhs.z); }

    Self &operator+=(const Self &rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
    Self &operator-=(const Self &rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
    Self &operator*=(const Self &rhs) noexcept { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
    Self &operator/=(const Self &rhs) noexcept { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

    bool operator==(const Self &rhs) const noexcept { return x == rhs.x && y == rhs.y && z == rhs.z; }
    bool operator!=(const Self &rhs) const noexcept { return x != rhs.x || y != rhs.y || z != rhs.z; }

    template<typename U>
    Self &operator+=(const U &rhs) noexcept { x += rhs; y += rhs; z += rhs; return *this; }
    template<typename U>
    Self &operator-=(const U &rhs) noexcept { x -= rhs; y -= rhs; z -= rhs; return *this; }
    template<typename U>
    Self &operator*=(const U &rhs) noexcept { x *= rhs; y *= rhs; z *= rhs; return *this; }
    template<typename U>
    Self &operator/=(const U &rhs) noexcept { x /= rhs; y /= rhs; z /= rhs; return *this; }

    static const Self &UNIT_X() noexcept
    {
        static const Self ret(T(1), T(0), T(0));
        return ret;
    }

    static const Self &UNIT_Y() noexcept
    {
        static const Self ret(T(0), T(1), T(0));
        return ret;
    }

    static const Self &UNIT_Z() noexcept
    {
        static const Self ret(T(0), T(0), T(1));
        return ret;
    }

    template<typename TDst>
    auto To() const noexcept(noexcept(TDst(x))) { return Vec3<TDst>(TDst(x), TDst(y), TDst(z)); }

    auto ToFloats() const noexcept(noexcept(this->To<float>())) { return To<float>(); }
    auto ToDoubles() const noexcept(noexcept(this->To<double>())) { return To<double>(); }

    auto LengthSquare()      const noexcept;
    auto Length()            const noexcept;
    Self Normalize()         const noexcept;
    Self Clamp(T min, T max) const noexcept;
    Self Sqrt()              const noexcept;

    Str8 ToString() const { return "(" + ToStr8(x) + ", " + ToStr8(y) + ", " + ToStr8(z) + ")"; }

    bool Serialize(BinarySerializer &serializer) const;
    bool Deserialize(BinaryDeserializer &deserializer);
};

template<typename T>
Vec3<T> operator-(const Vec3<T> &v) noexcept { return Vec3<T>(-v.x, -v.y, -v.z); }

template<typename T1, typename T2, std::enable_if_t<std::is_arithmetic_v<T1>, int> = 0>
Vec3<T2> operator+(const T1 &lhs, const Vec3<T2> &rhs) noexcept { return Vec3<T2>(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z); }
template<typename T1, typename T2, std::enable_if_t<std::is_arithmetic_v<T2>, int> = 0>
Vec3<T1> operator+(const Vec3<T1> &lhs, const T2 &rhs) noexcept { return Vec3<T1>(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs); }
template<typename T1, typename T2, std::enable_if_t<std::is_arithmetic_v<T2>, int> = 0>
Vec3<T1> operator-(const Vec3<T1> &lhs, const T2 &rhs) noexcept { return Vec3<T1>(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs); }
template<typename T1, typename T2>
Vec3<T2> operator*(const T1 &lhs, const Vec3<T2> &rhs) noexcept { return Vec3<T2>(T1(lhs) * rhs.x, T1(lhs) * rhs.y, T1(lhs) * rhs.z); }
template<typename T1, typename T2, std::enable_if_t<std::is_arithmetic_v<T2>, int> = 0>
Vec3<T1> operator*(const Vec3<T1> &lhs, const T2 &rhs) noexcept { return Vec3<T1>(T1(lhs.x * rhs), T1(lhs.y * rhs), T1(lhs.z * rhs)); }
template<typename T1, typename T2, std::enable_if_t<std::is_arithmetic_v<T2>, int> = 0>
Vec3<T1> operator/(const Vec3<T1> &lhs, const T2 &rhs) noexcept { return Vec3<T1>(lhs.x / T1(rhs), lhs.y / T1(rhs), lhs.z / T1(rhs)); }

template<typename T>
auto Cross(const Vec3<T> &lhs, const Vec3<T> &rhs) noexcept
{
    return Vec3<T>(lhs.y * rhs.z - lhs.z * rhs.y,
                   lhs.z * rhs.x - lhs.x * rhs.z,
                   lhs.x * rhs.y - lhs.y * rhs.x);
}

template<typename T1, typename T2>
auto Dot(const Vec3<T1> &lhs, const Vec3<T2> &rhs) noexcept
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template<typename T>
auto LengthSquare(const Vec3<T> &vec) noexcept
{
    return Dot(vec, vec);
}

template<typename T>
auto Length(const Vec3<T> &vec) noexcept
{
    return Sqrt(LengthSquare(vec));
}

template<typename T>
auto Normalize(const Vec3<T> &vec) noexcept
{
    return vec / Length(vec);
}

template<typename T>
auto Clamp(const Vec3<T> &vec, T minv, T maxv) noexcept
{
    return Vec3<decltype(Clamp(vec.x, minv, maxv))>(
                    Clamp(vec.x, minv, maxv),
                    Clamp(vec.y, minv, maxv),
                    Clamp(vec.z, minv, maxv));
}

template<typename T>
auto Sqrt(const Vec3<T> &v) noexcept
{
    return Vec3<decltype(Sqrt(v.x))>(Sqrt(v.x), Sqrt(v.y), Sqrt(v.y));
}

template<typename T>
bool ApproxEq(const Vec3<T> &lhs, const Vec3<T> &rhs, T epsilon) noexcept
{
    return ApproxEq(lhs.x, rhs.x, epsilon) &&
           ApproxEq(lhs.y, rhs.y, epsilon) &&
           ApproxEq(lhs.z, rhs.z, epsilon);
}

template<typename T>
auto Brightness(const Vec3<T> &v) noexcept
{
    return T(0.2126) * v.r + T(0.7152) * v.g + T(0.0722) * v.b;
}

template<typename T>
auto Vec3<T>::LengthSquare() const noexcept
{
    return ::AGZ::Math::LengthSquare(*this);
}

template<typename T>
auto Vec3<T>::Length() const noexcept
{
    return ::AGZ::Math::Length(*this);
}

template<typename T>
Vec3<T> Vec3<T>::Normalize() const noexcept
{
    return ::AGZ::Math::Normalize(*this);
}

template<typename T>
Vec3<T> Vec3<T>::Clamp(T min, T max) const noexcept
{
    return ::AGZ::Math::Clamp(*this, min, max);
}

template<typename T>
Vec3<T> Vec3<T>::Sqrt() const noexcept
{
    return ::AGZ::Math::Sqrt(*this);
}

template<typename T>
bool Vec3<T>::Serialize(BinarySerializer &serializer) const
{
    return serializer.Serialize(x) &&
           serializer.Serialize(y) &&
           serializer.Serialize(z);
}

template<typename T>
bool Vec3<T>::Deserialize(BinaryDeserializer &deserializer)
{
    return deserializer.Deserialize(x) &&
           deserializer.Deserialize(y) &&
           deserializer.Deserialize(z);
}

using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;

} // namespace AGZ::Math

namespace AGZ::Impl
{
    template<> struct ToImpl<char, Math::Vec3<float>>
    {
        static std::string Call(const Math::Vec3<float> &obj)
        {
            return "(" + std::to_string(obj.x) + ", "
                + std::to_string(obj.y) + ", "
                + std::to_string(obj.z) + ")";
        }
    };

} // namespace AGZ::Impl
