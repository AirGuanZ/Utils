#pragma once

#include "../Misc/Common.h"

namespace AGZ::Math {

template<typename T>
struct Deg;

template<typename T>
struct Rad
{
    T value;
    
    constexpr Rad() : value(T(0)) { }
    explicit constexpr Rad(T v) : value(v) { }
    explicit constexpr Rad(Uninitialized_t) { }
    constexpr Rad(const Deg<T> &d);
};

template<typename T>
struct Deg
{
    T value;

    constexpr Deg() : value(T(0)) { }
    explicit constexpr Deg(T v) : value(v) { }
    explicit constexpr Deg(Uninitialized_t) { }
    constexpr Deg(Rad<T> r)
        : value(T(180) / T(3.141592653589793238462643383) * r.value) { }
};

template<typename T>
constexpr Rad<T>::Rad(const Deg<T> &d)
    : value(T(3.141592653589793238462643383) / T(180) * d.value)
{
    
}

#define ANGLE_OPERATORS(Type) \
    template<typename T> constexpr auto operator+(Type<T> lhs, Type<T> rhs) \
        { return Type<T>(lhs.value + rhs.value); } \
    template<typename T> constexpr auto operator-(Type<T> lhs, Type<T> rhs) \
        { return Type<T>(lhs.value - rhs.value); } \
    template<typename T> constexpr auto operator-(Type<T> angle) \
        { return Type<T>(-angle.value); } \
    template<typename T> constexpr auto operator*(T lhs, Type<T> rhs) \
        { return Type<T>(lhs * rhs.value); } \
    template<typename T> constexpr auto operator*(Type<T> lhs, T rhs) \
        { return Type<T>(lhs.value * rhs); } \
    template<typename T> constexpr auto operator/(Type<T> lhs, T rhs) \
        { return Type<T>(lhs.value / rhs); }

ANGLE_OPERATORS(Rad)
ANGLE_OPERATORS(Deg)

using Degf = Deg<float>;
using Degd = Deg<double>;

using Radf = Rad<float>;
using Radd = Rad<double>;

} // namespace AGZ::Math
