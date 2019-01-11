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

    using Component = T;
    using Data      = T[2];
    using Self      = Vec2<T>;

    constexpr          Vec2()                noexcept : Vec2(T(0)) { }
    explicit constexpr Vec2(T value)         noexcept : x(value), y(value) { }
    constexpr          Vec2(T x, T y)        noexcept : x(x), y(y) { }
    explicit           Vec2(const T *data)   noexcept;
    explicit           Vec2(Uninitialized_t) noexcept { }

    Vec2(const Self &copyFrom)            noexcept: x(copyFrom.x), y(copyFrom.y) { }
    Self &operator=(const Self &copyFrom) noexcept { x = copyFrom.x; y = copyFrom.y; return *this; }

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

    static constexpr Self UNIT_X() noexcept { return Self(T(1), T(0)); }
    static constexpr Self UNIT_Y() noexcept { return Self(T(0), T(1)); }

    auto LengthSquare()      const noexcept;
    auto Length()            const noexcept;
    Self Normalize()         const noexcept;
    Self Clamp(T min, T max) const noexcept;

    Self Dot  (const Self &rhs) const noexcept;
    Self Cross(const Self &rhs) const noexcept;

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

template<typename T>
Vec2<T>::Vec2(const T *data) noexcept
{
    if constexpr(std::is_trivially_copyable_v<T>)
        std::memcpy(&x, data, sizeof(Data));
    else
    {
        x = data[0];
        y = data[1];
    }
}

template<typename T1, typename T2>
auto Dot(const Vec2<T1> &lhs, const Vec2<T2> &rhs) noexcept
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
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
    return x * x + y * y;
}

template<typename T>
auto Vec2<T>::Length() const noexcept
{
    return Sqrt(LengthSquare());
}

template<typename T>
Vec2<T> Vec2<T>::Normalize() const noexcept
{
    auto invLen = 1 / Length();
    return *this * invLen;
}

template<typename T>
Vec2<T> Vec2<T>::Clamp(T min, T max) const noexcept
{
    return { Math::Clamp(x, min, max), Math::Clamp(y, min, max) };
}

template<typename T>
Vec2<T> Vec2<T>::Dot(const Self &rhs) const noexcept
{
    return Dot(*this, rhs);
}

template<typename T>
Vec2<T> Vec2<T>::Cross(const Self &rhs) const noexcept
{
    return Cross(*this, rhs);
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
