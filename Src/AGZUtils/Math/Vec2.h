#pragma once

#include <cstring>
#include <type_traits>

#include "../Misc/Common.h"
#include "../Utils/Serialize.h"
#include "Scalar.h"

namespace AGZ::Math {

template<typename T>
class Vec2
{
public:

    union
    {
        struct { T u; T v; };
        struct { T x; T y; };
    };

    using Data = T[2];
    using Component = T;
    using Self = Vec2<T>;

    constexpr Vec2() noexcept : Vec2(T(0)) { }

    explicit Vec2(Uninitialized_t) noexcept { }

    explicit constexpr Vec2(T value) noexcept : x(value), y(value) { }

    constexpr Vec2(T x, T y) noexcept : x(x), y(y) { }

    Vec2(const Self &other) noexcept : x(other.x), y(other.y) { }

    explicit Vec2(const T *data) noexcept
    {
        static_assert(std::is_trivially_copyable_v<T>);
        std::memcpy(&x, data, sizeof(Data));
    }

    Self &operator=(const Self &other) noexcept
    {
        x = other.x;
        y = other.y;
        return *this;
    }

    template<typename F>
    auto Map(F &&f) const noexcept
    {
        using U = remove_rcv_t<decltype(f(x))>;
        return Vec2<U>(f(x), f(y));
    }

#include "SwizzleVec2.inl"

#define x u
#define y v

#include "SwizzleVec2.inl"

#undef x
#undef y

          T &operator[](size_t idx) noexcept { AGZ_ASSERT(idx < 2); return (&x)[idx]; }
    const T &operator[](size_t idx) const noexcept { AGZ_ASSERT(idx < 2); return (&x)[idx]; }

    Self operator+(const Self &rhs) const noexcept { return Self(x + rhs.x, y + rhs.y); }
    Self operator-(const Self &rhs) const noexcept { return Self(x - rhs.x, y - rhs.y); }
    Self operator*(const Self &rhs) const noexcept { return Self(x * rhs.x, y * rhs.y); }
    Self operator/(const Self &rhs) const noexcept { return Self(x / rhs.x, y / rhs.y); }

    Self &operator+=(const Self &rhs) noexcept { x += rhs.x; y += rhs.y; return *this; }
    Self &operator-=(const Self &rhs) noexcept { x -= rhs.x; y -= rhs.y; return *this; }
    Self &operator*=(const Self &rhs) noexcept { x *= rhs.x; y *= rhs.y; return *this; }
    Self &operator/=(const Self &rhs) noexcept { x /= rhs.x; y /= rhs.y; return *this; }

    bool operator==(const Self &rhs) const noexcept { return x == rhs.x && y == rhs.y; }
    bool operator!=(const Self &rhs) const noexcept { return x != rhs.x || y != rhs.y; }

    template<typename U>
    Self &operator+=(const U &rhs) noexcept { x += rhs; y += rhs; return *this; }
    template<typename U>
    Self &operator-=(const U &rhs) noexcept { x -= rhs; y -= rhs; return *this; }
    template<typename U>
    Self &operator*=(const U &rhs) noexcept { x *= rhs; y *= rhs; return *this; }
    template<typename U>
    Self &operator/=(const U &rhs) noexcept { x /= rhs; y /= rhs; return *this; }

    static const Self &UNIT_X() noexcept
    {
        static const Self ret(T(1), T(0));
        return ret;
    }

    static const Self &UNIT_Y() noexcept
    {
        static const Self ret(T(0), T(1));
        return ret;
    }

    auto LengthSquare()      const noexcept;
    auto Length()            const noexcept;
    Self Normalize()         const noexcept;
    Self Clamp(T min, T max) const noexcept;
    Self Sqrt()              const noexcept;

    bool Serialize(BinarySerializer &serializer) const noexcept;
    bool Deserialize(BinaryDeserializer &deserializer) noexcept;
};

template<typename T>
Vec2<T> operator-(const Vec2<T> &v) noexcept { return Vec2<T>(-v.x, -v.y); }

template<typename T1, typename T2>
Vec2<T2> operator+(const T1 &lhs, const Vec2<T2> &rhs) noexcept { return Vec2<T2>(lhs + rhs.x, lhs + rhs.y); }
template<typename T1, typename T2>
Vec2<T1> operator+(const Vec2<T1> &lhs, const T2 &rhs) noexcept { return Vec2<T1>(lhs.x + rhs, lhs.y + rhs); }
template<typename T1, typename T2>
Vec2<T1> operator-(const Vec2<T1> &lhs, const T2 &rhs) noexcept { return Vec2<T1>(lhs.x - rhs, lhs.y - rhs); }
template<typename T1, typename T2>
Vec2<T2> operator*(const T1 &lhs, const Vec2<T2> &rhs) noexcept { return Vec2<T2>(lhs * rhs.x, lhs * rhs.y); }
template<typename T1, typename T2>
Vec2<T1> operator*(const Vec2<T1> &lhs, const T2 &rhs) noexcept { return Vec2<T1>(lhs.x * rhs, lhs.y * rhs); }
template<typename T1, typename T2>
Vec2<T1> operator/(const Vec2<T1> &lhs, const T2 &rhs) noexcept { return Vec2<T1>(lhs.x / rhs, lhs.y / rhs); }

template<typename T1, typename T2>
auto Dot(const Vec2<T1> &lhs, const Vec2<T2> &rhs) noexcept
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

template<typename T>
auto LengthSquare(const Vec2<T> &vec) noexcept
{
    return Dot(vec, vec);
}

template<typename T>
auto Length(const Vec2<T> &vec) noexcept
{
    return Sqrt(LengthSquare(vec));
}

template<typename T>
auto Normalize(const Vec2<T> &vec) noexcept
{
    return vec / Length(vec);
}

template<typename T>
auto Clamp(const Vec2<T> &vec, T minv, T maxv) noexcept
{
    return Vec2<decltype(Clamp(vec.x, minv, maxv))>(
        Clamp(vec.x, minv, maxv), Clamp(vec.y, minv, maxv));
}

template<typename T>
auto Sqrt(const Vec2<T> &v) noexcept
{
    return Vec2<decltype(Sqrt(v.x))>(Sqrt(v.x), Sqrt(v.y));
}

template<typename T>
bool ApproxEq(const Vec2<T> &lhs, const Vec2<T> &rhs, T epsilon) noexcept
{
    return ApproxEq(lhs.x, rhs.x, epsilon) &&
           ApproxEq(lhs.y, rhs.y, epsilon);
}

template<typename T>
auto Vec2<T>::LengthSquare() const noexcept
{
    return ::AGZ::Math::LengthSquare(*this);
}

template<typename T>
auto Vec2<T>::Length() const noexcept
{
    return ::AGZ::Math::Length(*this);
}

template<typename T>
Vec2<T> Vec2<T>::Normalize() const noexcept
{
    return ::AGZ::Math::Normalize(*this);
}

template<typename T>
Vec2<T> Vec2<T>::Clamp(T min, T max) const noexcept
{
    return ::AGZ::Math::Clamp(*this, min, max);
}

template<typename T>
Vec2<T> Vec2<T>::Sqrt() const noexcept
{
    return ::AGZ::Math::Sqrt(*this);
}

template<typename T>
bool Vec2<T>::Serialize(BinarySerializer &serializer) const noexcept
{
    return serializer.Serialize(x) &&
           serializer.Serialize(y);
}

template<typename T>
bool Vec2<T>::Deserialize(BinaryDeserializer &deserializer) noexcept
{
    return deserializer.Deserialize(x) &&
           deserializer.Deserialize(y);
}

using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;

} // namespace AGZ::Math
