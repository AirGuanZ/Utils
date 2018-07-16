#pragma once

#include "../Common.h"

AGZ_NS_BEG(AGZ::Math)

template<typename T, int I>
struct AngleTemplate
{
    T value;

    AngleTemplate() : value(T(0.0)) { }
    explicit AngleTemplate(T v) : value(v) { }
    explicit AngleTemplate(Uninitialized_t) { }
};

template<typename T>
using Rad = AngleTemplate<T, 0>;

template<typename T>
using Deg = AngleTemplate<T, 1>;

template<typename T, int I>
AGZ_FORCE_INLINE auto operator+(AngleTemplate<T, I> lhs, AngleTemplate<T, I> rhs)
{
    return AngleTemplate<T, I>{ lhs.value + rhs.value };
}

template<typename T, int I>
AGZ_FORCE_INLINE auto operator-(AngleTemplate<T, I> lhs, AngleTemplate<T, I> rhs)
{
    return AngleTemplate<T, I>{ lhs.value - rhs.value };
}

template<typename T, int I>
AGZ_FORCE_INLINE auto operator*(AngleTemplate<T, I> lhs, T rhs)
{
    return AngleTemplate<T, I>{ lhs.value * rhs };
}

template<typename T, int I>
AGZ_FORCE_INLINE auto operator*(T lhs, AngleTemplate<T, I> rhs)
{
    return rhs * lhs;
}

template<typename T, int I>
AGZ_FORCE_INLINE auto operator/(AngleTemplate<T, I> lhs, T rhs)
{
    return AngleTemplate<T, I>{ lhs.value / rhs };
}

using Degf = Deg<float>;
using Degd = Deg<double>;

using Radf = Rad<float>;
using Radd = Rad<double>;

AGZ_NS_END(AGZ::Math)
