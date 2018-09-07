#pragma once

#include <cmath>
#include <algorithm>
#include <limits>
#include <type_traits>

#include "../Misc/Common.h"
#include "../Misc/TypeOpr.h"
#include "Angle.h"

AGZ_NS_BEG(AGZ::Math)

template<typename T> struct Abs_impl;
template<>           struct Abs_impl<float>  { static float Abs(float v)   { return std::abs(v); } };
template<>           struct Abs_impl<double> { static double Abs(double v) { return std::fabs(v); } };
template<typename T> struct Abs_impl<Rad<T>> { static Rad<T> Abs(Rad<T> v) { return Rad<T>{ Abs_impl<T>::Abs(v.value) }; } };
template<typename T> struct Abs_impl<Deg<T>> { static Deg<T> Abs(Deg<T> v) { return Deg<T>{ Abs_impl<T>::Abs(v.value) }; } };

template<typename T> T Abs(T v) { return Abs_impl<T>::Abs(v); }

template<typename T>        T      Sqrt(T);
template<>           inline float  Sqrt<float>(float value)   { return std::sqrt(value); }
template<>           inline double Sqrt<double>(double value) { return std::sqrt(value); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
T Clamp(T v, T minv, T maxv) { return (std::max)((std::min)(v, maxv), minv); }

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
// See https://randomascii.wordpress.com/category/floating-point/
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

private:

    static constexpr size_t BIT_COUNT      = 8 * sizeof(ValueType);
    static constexpr size_t FRAC_BIT_COUNT = std::numeric_limits<F>::digits - 1;
    static constexpr size_t EXPT_BIT_COUNT = BIT_COUNT - 1 - FRAC_BIT_COUNT;

    static constexpr size_t SIGN_BIT_MASK = static_cast<InternalUInt>(1) << (BIT_COUNT - 1);
    static constexpr size_t FRAC_BIT_MASK = ~static_cast<InternalUInt>(0) >> (EXPT_BIT_COUNT + 1);
    static constexpr size_t EXPT_BIT_MASK = ~(SIGN_BIT_MASK | FRAC_BIT_MASK);

    static constexpr size_t DEFAULT_MAX_ULP = 4;

    union
    {
        ValueType float_;
        InternalUInt uint_;
    };

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

    static InternalUInt SAM2Biased(InternalUInt sam)
    {
        return (SIGN_BIT_MASK & sam) ? (~sam + 1) : (SIGN_BIT_MASK | sam);
    }

    static InternalUInt DistanceBetweenSAMs(InternalUInt lhs, InternalUInt rhs)
    {
        auto blhs = SAM2Biased(lhs), brhs = SAM2Biased(rhs);
        return (blhs >= brhs) ? (blhs - brhs) : (brhs - blhs);
    }

public:

    constexpr FP() : float_(F(0)) { }
    constexpr FP(F v) : float_(v) { }
    FP(Uninitialized_t) { }

    static Self Infinity() { return Bits2Value(EXPT_BIT_MASK); }
    static Self Max()      { return std::numeric_limits<ValueType>::max(); }

    ValueType Value()   const { return float_; }
    InternalUInt Bits() const { return uint_; }

    InternalUInt ExptBits() const { return EXPT_BIT_MASK & uint_; }
    InternalUInt FracBits() const { return FRAC_BIT_MASK & uint_; }
    InternalUInt SignBit()  const { return SIGN_BIT_MASK & uint_; }

    bool IsNAN()      const { return ExptBits() == EXPT_BIT_MASK && FracBits(); }
    bool IsInfinity() const { return ExptBits() == EXPT_BIT_MASK && !FracBits(); }

    static constexpr size_t DefaultEqEpsilon() { return DEFAULT_MAX_ULP; }

    bool ApproxEq(const Self &rhs, size_t maxULP = DEFAULT_MAX_ULP) const
    {
        return !IsNAN() && !rhs.IsNAN() &&
                DistanceBetweenSAMs(uint_, rhs.uint_) <= maxULP;
    }
};

using Float  = FP<float>;
using Double = FP<double>;

AGZ_NS_END(AGZ::Math)
