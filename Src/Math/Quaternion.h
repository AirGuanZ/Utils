#pragma once

#include "../Misc/Common.h"
#include "Vec3.h"

namespace AGZ::Math {

template<typename T>
class Quaternion
{
public:

    Vec3<T> u;
    T w;

    using Component = T;
    using Self = Quaternion<T>;

    constexpr Quaternion() : u(), w(T(0)) { }

    explicit Quaternion(Uninitialized_t) { }

    constexpr Quaternion(T x, T y, T z, T w) : u(x, y, z), w(w) { }

    Quaternion(const Vec3<T> &u, T w) : u(u), w(w) { }

    Quaternion(const Self &)      = default;
    Self &operator=(const Self &) = default;
    ~Quaternion()                 = default;

    template<typename U>
    static Self Rotate(const Vec3<T> &axis, U angle)
    {
        auto a = T(0.5) * angle;
        return Quaternion<T>(Sin(a) * axis, Cos(a));
    }

    Self operator+(const Self &rhs) const { return Self(u + rhs.u, w + rhs.w); }
    Self operator-(const Self &rhs) const { return Self(u - rhs.u, w - rhs.w); }

    Self operator*(const Self &rhs) const { return Self(w * rhs.u + rhs.w * u + Cross(u, rhs.u),
                                                        w * rhs.w - Dot(u, rhs.u)); }

    Self operator*(Component v) const { return Self(v * u, v * w); }
    Self operator/(Component v) const { return Self(u / v, w / v); }

    Self &operator+=(const Self &rhs) { w += rhs.w; u += rhs.u; return *this; }
    Self &operator-=(const Self &rhs) { w -= rhs.w; u -= rhs.u; return *this; }
    Self &operator*=(const Self &rhs) { return *this = *this * rhs; }

    Self &operator*=(Component v) { u *= v; w *= v; return *this; }
    Self &operator/=(Component v) { u /= v; w /= v; return *this; }

    bool operator==(const Self &rhs) const { return u == rhs.u && w == rhs.w; }
    bool operator!=(const Self &rhs) const { return u != rhs.u || w != rhs.w; }
};

template<typename T>
Quaternion<T> operator-(const Quaternion<T> &q) { return Quaternion<T>(-q.u, -q.w); }

template<typename T>
Quaternion<T> Conjugate(const Quaternion<T> &q) { return Quaternion<T>(-q.u, q.w); }

template<typename T>
auto LengthSquare(const Quaternion<T> &q) { return LengthSquare(q.u) + q.w * q.w; }

template<typename T>
auto Length(const Quaternion<T> &q) { return Sqrt(LengthSquare(q)); }

template<typename T>
Quaternion<T> Inverse(const Quaternion<T> &q) { return Conjugate(q) / LengthSquare(q); }

template<typename T>
Quaternion<T> operator*(T v, const Quaternion<T> &q) { return q * v; }

template<typename T>
Vec3<T> Apply(const Quaternion<T> &q, const Vec3<T> &v)
{
    return (q * Quaternion<T>(v, T(0)) * Conjugate(q)).u;
}

using Quaternionf = Quaternion<float>;
using Quaterniond = Quaternion<double>;

} // namespace AGZ::Math
