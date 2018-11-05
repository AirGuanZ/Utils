#pragma once

#include <cmath>
#include <algorithm>
#include <limits>
#include <type_traits>

#include "../Misc/Common.h"
#include "../Misc/TypeOpr.h"
#include "Angle.h"

namespace AGZ::Math {

template<typename T> struct Abs_impl;
template<>           struct Abs_impl<float>  { static float  Abs(float v)   { return std::abs(v); } };
template<>           struct Abs_impl<double> { static double Abs(double v) { return std::fabs(v); } };
template<typename T> struct Abs_impl<Rad<T>> { static Rad<T> Abs(Rad<T> v) { return Rad<T>{ Abs_impl<T>::Abs(v.value) }; } };
template<typename T> struct Abs_impl<Deg<T>> { static Deg<T> Abs(Deg<T> v) { return Deg<T>{ Abs_impl<T>::Abs(v.value) }; } };

template<typename T> T Abs(T v) { return Abs_impl<T>::Abs(v); }

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
auto Sqrt(T value) { return std::sqrt(value); }

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
auto Exp(T value) { return std::exp(value); }

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
auto Log_e(T value) { return std::log(value); }

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
auto Log_2(T value) { return std::log2(value); }

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
auto Log_10(T value) { return std::log10(value); }

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
auto Pow(T x, T y) { return std::pow(x, y); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
T Clamp(T v, T minv, T maxv) { return (std::max)((std::min)(v, maxv), minv); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
T Min(T lhs, T rhs) { return (std::min)(lhs, rhs); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
T Max(T lhs, T rhs) { return (std::max)(lhs, rhs); }

template<typename T, typename U = decltype(T::DefaultEqEpsilon()),
    std::enable_if_t<TypeOpr::True_v<decltype(&T::ApproxEq)>, int> = 0>
AGZ_FORCEINLINE bool ApproxEq(const T &lhs, const T &rhs, U epsilon = T::DefaultEqEpsilon())
{
    return lhs.ApproxEq(rhs, epsilon);
}

template<typename T, typename U, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
AGZ_FORCEINLINE bool ApproxEq(T lhs, T rhs, U epsilon)
{
    return Abs(lhs - rhs) <= epsilon;
}

// IEEE754 floating-poing number
// See https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
template<typename F, std::enable_if_t<(std::is_floating_point_v<F> &&
                                       !std::is_same_v<F, long double>),
                                      int> = 0>
class FP
{
public:
    using Self         = FP<F>;
    using ValueType    = F;
    using InternalUInt = std::conditional_t<sizeof(F) == sizeof(uint32_t),
                                            uint32_t, uint64_t>;

    union
    {
        ValueType float_;
        InternalUInt uint_;
    };

private:

    static constexpr size_t BIT_COUNT      = 8 * sizeof(ValueType);
    static constexpr size_t FRAC_BIT_COUNT = std::numeric_limits<F>::digits - 1;
    static constexpr size_t EXPT_BIT_COUNT = BIT_COUNT - 1 - FRAC_BIT_COUNT;

    static constexpr InternalUInt SIGN_BIT_MASK = static_cast<InternalUInt>(1) << (BIT_COUNT - 1);
    static constexpr InternalUInt FRAC_BIT_MASK = ~static_cast<InternalUInt>(0) >> (EXPT_BIT_COUNT + 1);
    static constexpr InternalUInt EXPT_BIT_MASK = ~(SIGN_BIT_MASK | FRAC_BIT_MASK);

    static constexpr size_t DEFAULT_MAX_ULP = 4;

    static ValueType Bits2Value(InternalUInt uint)
    {
        Self ret(UNINITIALIZED);
        ret.uint_ = uint;
        return ret.float_;
    }

    static InternalUInt Value2Bits(ValueType v)
    {
        Self ret(v);
        return ret.uint_;
    }

public:

    constexpr FP() : float_(F(0)) { }
    constexpr FP(F v) : float_(v) { }
    explicit FP(Uninitialized_t) { }

    static Self Infinity() { return Bits2Value(EXPT_BIT_MASK); }
    static Self Max()      { return (std::numeric_limits<ValueType>::max)(); }
    static Self Min()      { return (std::numeric_limits<ValueType>::lowest()); }

    ValueType Value()   const { return float_; }
    InternalUInt Bits() const { return uint_; }

    InternalUInt ExptBits() const { return EXPT_BIT_MASK & uint_; }
    InternalUInt FracBits() const { return FRAC_BIT_MASK & uint_; }
    InternalUInt SignBit()  const { return SIGN_BIT_MASK & uint_; }

    operator ValueType() const { return Value(); }

    bool IsNAN()      const { return ExptBits() == EXPT_BIT_MASK && FracBits(); }
    bool IsInfinity() const { return ExptBits() == EXPT_BIT_MASK && !FracBits(); }
    bool IsNegative() const { return SignBit() != 0; }

    static constexpr size_t DefaultEqEpsilon() { return DEFAULT_MAX_ULP; }

    bool ApproxEq(Self rhs, size_t maxULPs = DEFAULT_MAX_ULP) const
    {
        if(IsNegative() != rhs.IsNegative())
            return float_ == rhs.float_;
        InternalUInt ULPsDiff = uint_ > rhs.uint_ ? uint_ - rhs.uint_ : rhs.uint_ - uint_;
        return ULPsDiff <= maxULPs;
    }

#define FP_BINARY_OPERATOR(binary_opr, assign_opr) \
    auto operator binary_opr(Self rhs) const { return Self(float_ binary_opr rhs.float_); } \
    Self &operator assign_opr(Self rhs) { float_ assign_opr rhs,float_; return *this; }

    FP_BINARY_OPERATOR(+, +=)
    FP_BINARY_OPERATOR(-, -=)
    FP_BINARY_OPERATOR(*, *=)
    FP_BINARY_OPERATOR(/, /=)

#undef FP_BINARY_OPERATOR

#define FP_COMPARE_OPERATOR(comp_opr) \
    bool operator comp_opr(Self rhs) const { return float_ comp_opr rhs.float_; }

    FP_COMPARE_OPERATOR(==)
    FP_COMPARE_OPERATOR(!=)
    FP_COMPARE_OPERATOR(<)
    FP_COMPARE_OPERATOR(>)
    FP_COMPARE_OPERATOR(<=)
    FP_COMPARE_OPERATOR(>=)

#undef FP_COMPARE_OPERATOR

    Self operator-() const { return Self(-float_); }
};

using Float  = FP<float>;
using Double = FP<double>;

} // namespace AGZ::Math
