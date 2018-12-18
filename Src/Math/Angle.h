#pragma once

/**
 * @file Math/Angle.h
 * @brief 将rad/deg两种单位的区别以类型的方式给出
 * 
 * 提供Rad<T>和Deg<T>两种类型，它们之间可以自由地进行转换，转换时会根据类型所代表的单位自动调整内部的值
 */

#include "../Misc/Common.h"

namespace AGZ::Math {

/**
 * @brief 角度的公共基类，可用该类型结合SFINAE来过滤角度类型
 * @see IsAngleType_v
 */
struct AngleBase { };

/**
 * @brief 某个类型是否是Rad/Deg类型
 */
template<typename T>
constexpr bool IsAngleType_v = std::is_base_of_v<AngleBase, T>;

template<typename T>
struct Deg;

/**
 * @brief 弧度值
 * 
 * 可进行基本的算术运算和三角函数运算
 */
template<typename T>
struct Rad : AngleBase
{
    T value;

    /** 默认初始化为零 */
    constexpr Rad() noexcept: value(T(0)) { }
    /** 初始化为指定的弧度值 */
    explicit constexpr Rad(T v) noexcept: value(v) { }
    /** 不初始化内部值 */
    explicit constexpr Rad(Uninitialized_t) noexcept { }
    /** 从角度转换而来 */
    constexpr Rad(const Deg<T> &d) noexcept;
};

/**
 * @brief 角度值
 * 
 * 可进行基本的算术运算和三角函数运算
 */
template<typename T>
struct Deg : AngleBase
{
    T value;

    /** 默认初始化为零 */
    constexpr Deg() noexcept: value(T(0)) { }
    /** 初始化为指定的角度值 */
    explicit constexpr Deg(T v) noexcept: value(v) { }
    /** 不初始化内部值 */
    explicit constexpr Deg(Uninitialized_t) noexcept { }
    /** 从弧度转换而来 */
    constexpr Deg(Rad<T> r) noexcept
        : value(T(180) / T(3.141592653589793238462643383) * r.value) { }
};

template<typename T>
constexpr Rad<T>::Rad(const Deg<T> &d) noexcept
    : value(T(3.141592653589793238462643383) / T(180) * d.value)
{
    
}

#define ANGLE_OPERATORS(Type) \
    template<typename T> constexpr auto operator+(Type<T> lhs, Type<T> rhs) noexcept \
        { return Type<T>(lhs.value + rhs.value); } \
    template<typename T> constexpr auto operator-(Type<T> lhs, Type<T> rhs) noexcept \
        { return Type<T>(lhs.value - rhs.value); } \
    template<typename T> constexpr auto operator-(Type<T> angle) noexcept \
        { return Type<T>(-angle.value); } \
    template<typename T> constexpr auto operator*(T lhs, Type<T> rhs) noexcept \
        { return Type<T>(lhs * rhs.value); } \
    template<typename T> constexpr auto operator*(Type<T> lhs, T rhs) noexcept \
        { return Type<T>(lhs.value * rhs); } \
    template<typename T> constexpr auto operator/(Type<T> lhs, T rhs) noexcept \
        { return Type<T>(lhs.value / rhs); }

ANGLE_OPERATORS(Rad)
ANGLE_OPERATORS(Deg)

#undef ANGLE_OPERATORS

using Degf = Deg<float>;
using Degd = Deg<double>;

using Radf = Rad<float>;
using Radd = Rad<double>;

} // namespace AGZ::Math
