﻿#pragma once

/**
 * @file Math/Quaternion.h
 * @brief 定义了用于旋转变换的四元数类型
 */

#include "../Misc/Common.h"
#include "Vec3.h"

namespace AGZ::Math {

/**
 * @brief 四元数类型
 * 
 * @see https://airguanz.github.io/2018/09/04/quaternion.html
 */
template<typename T>
class Quaternion
{
public:

    Vec3<T> u;
    T w;

    using Component = T;
    using Self = Quaternion<T>;

    /** 默认初始化为零四元数 */
    constexpr Quaternion() noexcept: u(), w(T(0)) { }

    /** 不进行任何初始化 */
    explicit Quaternion(Uninitialized_t) noexcept { }

    /** 分别以指定值初始化各元素 */
    constexpr Quaternion(T x, T y, T z, T w) noexcept: u(x, y, z), w(w) { }

    /** 分别以指定值初始化各元素 */
    Quaternion(const Vec3<T> &u, T w) noexcept: u(u), w(w) { }

    Quaternion(const Self &)      = default;
    Self &operator=(const Self &) = default;
    ~Quaternion()                 = default;

    /** 构建表示绕指定轴旋转一定角度的四元数 */
    template<typename U, std::enable_if_t<IsAngleType_v<U>, int> = 0>
    static Self Rotate(const Vec3<T> &axis, U angle) noexcept
    {
        auto a = T(0.5) * angle;
        return Quaternion<T>(Sin(a) * axis.Normalize(), Cos(a));
    }

    /** 将旋转作用到给定向量上 */
    Vec3<T> Apply(const Vec3<T> &v) const noexcept;

    /** 逐元素相加 */
    Self operator+(const Self &rhs) const noexcept { return Self(u + rhs.u, w + rhs.w); }
    /** 逐元素相减 */
    Self operator-(const Self &rhs) const noexcept { return Self(u - rhs.u, w - rhs.w); }

    /** 四元数乘法 */
    Self operator*(const Self &rhs) const noexcept { return Self(w * rhs.u + rhs.w * u + Cross(u, rhs.u),
                                                        w * rhs.w - Dot(u, rhs.u)); }

    /** 标量乘法 */
    Self operator*(Component v) const noexcept { return Self(v * u, v * w); }
    /** 标量除法 */
    Self operator/(Component v) const noexcept { return Self(u / v, w / v); }

    /** 逐元素相加 */
    Self &operator+=(const Self &rhs) noexcept { w += rhs.w; u += rhs.u; return *this; }
    /** 逐元素相减 */
    Self &operator-=(const Self &rhs) noexcept { w -= rhs.w; u -= rhs.u; return *this; }
    /** 四元数乘法 */
    Self &operator*=(const Self &rhs) noexcept { return *this = *this * rhs; }

    /** 标量乘法 */
    Self &operator*=(Component v) noexcept { u *= v; w *= v; return *this; }
    /** 标量除法 */
    Self &operator/=(Component v) noexcept { u /= v; w /= v; return *this; }

    /** 是否逐元素相等 */
    bool operator==(const Self &rhs) const noexcept { return u == rhs.u && w == rhs.w; }
    /** 是否有元素不等 */
    bool operator!=(const Self &rhs) const noexcept { return u != rhs.u || w != rhs.w; }
};

template<typename T>
Quaternion<T> operator-(const Quaternion<T> &q) noexcept { return Quaternion<T>(-q.u, -q.w); }

/** 求给定四元数的共轭 */
template<typename T>
Quaternion<T> Conjugate(const Quaternion<T> &q) noexcept { return Quaternion<T>(-q.u, q.w); }

/** 求给定四元数的模长的平方 */
template<typename T>
auto LengthSquare(const Quaternion<T> &q) noexcept { return LengthSquare(q.u) + q.w * q.w; }

/** 求给定四元数的模 */
template<typename T>
auto Length(const Quaternion<T> &q) noexcept { return Sqrt(LengthSquare(q)); }

/** 求给定四元数的逆 */
template<typename T>
Quaternion<T> Inverse(const Quaternion<T> &q) noexcept { return Conjugate(q) / LengthSquare(q); }

/** 标量乘法 */
template<typename T>
Quaternion<T> operator*(T v, const Quaternion<T> &q) noexcept { return q * v; }

/** 将四元数所代表的旋转作用在向量上 */
template<typename T>
Vec3<T> Apply(const Quaternion<T> &q, const Vec3<T> &v) noexcept
{
    return (q * Quaternion<T>(v, T(0)) * Conjugate(q)).u;
}

template <typename T>
Vec3<T> Quaternion<T>::Apply(const Vec3<T> &v) const noexcept
{
    return ::AGZ::Math::Apply(*this, v);
}

using Quaternionf = Quaternion<float>;
using Quaterniond = Quaternion<double>;

} // namespace AGZ::Math
