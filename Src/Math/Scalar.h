#pragma once

#include <cmath>
#include <algorithm>
#include <limits>

#include "../Misc/Common.h"
#include "../Misc/TypeOpr.h"
#include "Angle.h"

namespace AGZ::Math {

namespace Impl
{
    template<typename T> struct Abs_impl { static T Abs(T v) noexcept { return std::abs(v); } };
    template<typename T> struct Abs_impl<Rad<T>> { static Rad<T> Abs(Rad<T> v) noexcept { return Rad<T>{ Abs_impl<T>::Abs(v.value) }; } };
    template<typename T> struct Abs_impl<Deg<T>> { static Deg<T> Abs(Deg<T> v) noexcept { return Deg<T>{ Abs_impl<T>::Abs(v.value) }; } };
}

template<typename T>
T Abs(T v) noexcept { return Impl::Abs_impl<T>::Abs(v); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
auto Reciprocate(T value) noexcept { return T(1) / value; }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
auto Sqrt(T value) noexcept(noexcept(std::sqrt(value))) { return std::sqrt(value); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
auto Exp(T value) noexcept(noexcept(std::exp(value))) { return std::exp(value); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
auto Log_e(T value) noexcept(noexcept(std::log(value))) { return std::log(value); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
auto Log_2(T value) noexcept(noexcept(std::log2(value))) { return std::log2(value); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
auto Log_10(T value) noexcept(noexcept(std::log10(value))) { return std::log10(value); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
auto Pow(T x, T y) noexcept(noexcept(std::pow(x, y))) { return std::pow(x, y); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
T Clamp(T v, T minv, T maxv) noexcept { return (std::max)((std::min)(v, maxv), minv); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
T ClampToPositive(T v) noexcept { return (std::max)(T(0), v); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
T Saturate(T v) { return Clamp<T>(v, 0, 1); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
T Min(T lhs, T rhs) noexcept { return (std::min)(lhs, rhs); }

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
T Max(T lhs, T rhs) noexcept { return (std::max)(lhs, rhs); }

template<typename T, typename U = decltype(T::DefaultEqEpsilon()),
    std::enable_if_t<TypeOpr::True_v<decltype(&T::ApproxEq)>, int> = 0>
bool ApproxEq(const T &lhs, const T &rhs, U epsilon = T::DefaultEqEpsilon()) noexcept(noexcept(lhs.ApproxEq(rhs, epsilon)))
{
    return lhs.ApproxEq(rhs, epsilon);
}

template<typename T, typename U, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
bool ApproxEq(T lhs, T rhs, U epsilon) noexcept
{
    return Abs(lhs - rhs) <= epsilon;
}

namespace Impl
{
    template<typename T> struct PI_impl;
    template<>           struct PI_impl<float>  { static constexpr float  PI() noexcept { return 3.141592653589793238462643383f; } };
    template<>           struct PI_impl<double> { static constexpr double PI() noexcept { return 3.141592653589793238462643383; } };
    template<typename T> struct PI_impl<Rad<T>> { static constexpr Rad<T> PI() noexcept { return Rad<T>{ PI_impl<T>::PI() }; } };
    template<typename T> struct PI_impl<Deg<T>> { static constexpr Deg<T> PI() noexcept { return Deg<T>{ T(180.0) }; } };

    template<typename T> T Sin_rawimpl(T);
    template<>           inline float  Sin_rawimpl<float>(float rad) noexcept(noexcept(std::sin(rad))) { return std::sin(rad); }
    template<>           inline double Sin_rawimpl<double>(double rad) noexcept(noexcept(std::sin(rad))) { return std::sin(rad); }

    template<typename T> T Cos_rawimpl(T);
    template<>           inline float  Cos_rawimpl<float>(float rad)   noexcept(noexcept(std::cos(rad))) { return std::cos(rad); }
    template<>           inline double Cos_rawimpl<double>(double rad) noexcept(noexcept(std::cos(rad))) { return std::cos(rad); }

    template<typename T> struct Sin_impl;
    template<>           struct Sin_impl<float>  { static auto Sin(float rad)  noexcept(noexcept(Sin_rawimpl<float>(rad))) { return Sin_rawimpl<float>(rad); } };
    template<>           struct Sin_impl<double> { static auto Sin(double rad) noexcept(noexcept(Sin_rawimpl<double>(rad))) { return Sin_rawimpl<double>(rad); } };
    template<typename T> struct Sin_impl<Rad<T>> { static auto Sin(Rad<T> rad) noexcept(noexcept(Sin_rawimpl<T>(rad.value))) { return Sin_rawimpl<T>(rad.value); } };
    template<typename T> struct Sin_impl<Deg<T>> { static auto Sin(Deg<T> deg) noexcept(noexcept(Sin_rawimpl<T>(deg.value * (PI_impl<T>::PI() / T(180.0))))) { return Sin_rawimpl<T>(deg.value * (PI_impl<T>::PI() / T(180.0))); } };

    template<typename T> struct Cos_impl;
    template<>           struct Cos_impl<float> { static auto Cos(float rad)   noexcept(noexcept(Cos_rawimpl<float>(rad))) { return Cos_rawimpl<float>(rad); } };
    template<>           struct Cos_impl<double> { static auto Cos(double rad) noexcept(noexcept(Cos_rawimpl<double>(rad))) { return Cos_rawimpl<double>(rad); } };
    template<typename T> struct Cos_impl<Rad<T>> { static auto Cos(Rad<T> rad) noexcept(noexcept(Cos_rawimpl<T>(rad.value))) { return Cos_rawimpl<T>(rad.value); } };
    template<typename T> struct Cos_impl<Deg<T>> { static auto Cos(Deg<T> deg) noexcept(noexcept(Cos_rawimpl<T>(deg.value * (PI_impl<T>::PI() / T(180.0))))) { return Cos_rawimpl<T>(deg.value * (PI_impl<T>::PI() / T(180.0))); } };
}

template<typename T> constexpr auto Deg2Rad(Deg<T> deg) noexcept { return Rad<T> { deg.value * (Impl::PI_impl<T>::PI() / T(180.0)) }; }
template<typename T> constexpr auto Rad2Deg(Rad<T> rad) noexcept { return Deg<T> { rad.value * (T(180.0) / Impl::PI_impl<T>::PI()) }; }

template<typename T> constexpr auto AsDeg(Deg<T> deg) noexcept { return deg; }
template<typename T> constexpr auto AsDeg(Rad<T> rad) noexcept { return Rad2Deg<T>(rad); }

template<typename T> constexpr auto AsRad(Deg<T> deg) noexcept { return Deg2Rad<T>(deg); }
template<typename T> constexpr auto AsRad(Rad<T> rad) noexcept { return rad; }

template<typename T>
constexpr auto PI = Impl::PI_impl<T>::PI();
template<typename T>
constexpr auto PIx2 = T(2) * PI<T>;
template<typename T>
constexpr auto PIx4 = T(4) * PI<T>;
template<typename T>
constexpr auto InvPI = T(1) / PI<T>;
template<typename T>
constexpr auto Inv2PI = T(1) / PIx2<T>;
template<typename T>
constexpr auto Inv4PI = T(1) / PIx4<T>;

template<typename T> auto Sin(T angle) noexcept(noexcept(Impl::Sin_impl<T>::Sin(angle))) { return Impl::Sin_impl<T>::Sin(angle); }
template<typename T> auto Cos(T angle) noexcept(noexcept(Impl::Cos_impl<T>::Cos(angle))) { return Impl::Cos_impl<T>::Cos(angle); }
template<typename T> auto Tan(T angle) noexcept(noexcept(Sin<T>(angle)) && noexcept(Cos<T>(angle))) { return Sin<T>(angle) / Cos<T>(angle); }
template<typename T> auto Cot(T angle) noexcept(noexcept(Sin<T>(angle)) && noexcept(Cos<T>(angle))) { return Cos<T>(angle) / Sin<T>(angle); }

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
T Arcsin(T v) { return std::asin(v); }
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
T Arccos(T v) { return std::acos(v); }
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
T Arctan(T v) { return std::atan(v); }
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
T Arctan2(T y, T x) { return std::atan2(y, x); }

namespace Impl
{
    template<typename T> struct OneMinusEpsilonImpl { };
    template<> struct OneMinusEpsilonImpl<float>
    {
        static constexpr float Value = 0x1.fffffep-1;
    };
    template<> struct OneMinusEpsilonImpl<double>
    {
        static constexpr double Value = 0x1.fffffffffffffp-1;
    };
}

/**
 * @brief IEEE754 floating-poing number
 * 
 * See https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
 */
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

    union // 理论上这是UB，但实际上works well
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

    static ValueType Bits2Value(InternalUInt uint) noexcept
    {
        Self ret(UNINITIALIZED);
        ret.uint_ = uint;
        return ret.float_;
    }

    static InternalUInt Value2Bits(ValueType v) noexcept
    {
        Self ret(v);
        return ret.uint_;
    }

public:

    constexpr FP() noexcept: float_(F(0)) { }
    constexpr FP(F v) noexcept: float_(v) { }
    explicit FP(Uninitialized_t) noexcept { }

    /**
     * 正无穷
     */
    static Self Infinity() noexcept { return Bits2Value(EXPT_BIT_MASK); }
    
    /**
     * 比1略小的值
     */
    static Self OneMinusEpsilon() noexcept { return Impl::OneMinusEpsilonImpl<F>::Value; }

    /**
     * 最大值
     */
    static Self Max() noexcept { return (std::numeric_limits<ValueType>::max)(); }
    
    /**
     * 最小值
     */
    static Self Min() noexcept { return (std::numeric_limits<ValueType>::lowest()); }

    /**
     * 取得浮点值
     */
    ValueType Value() const noexcept { return float_; }

    /**
     * 取得无符号整数表示
     */
    InternalUInt Bits() const noexcept { return uint_; }

    /**
     * 取得尾数
     */
    InternalUInt ExptBits() const noexcept { return EXPT_BIT_MASK & uint_; }
    
    /**
     * 取得阶码
     */
    InternalUInt FracBits() const noexcept { return FRAC_BIT_MASK & uint_; }
    
    /**
     * 取得符号位
     */
    InternalUInt SignBit()  const noexcept { return SIGN_BIT_MASK & uint_; }

    operator ValueType() const noexcept { return Value(); }

    /**
     * 是否是NAN
     */
    bool IsNAN() const noexcept { return ExptBits() == EXPT_BIT_MASK && FracBits(); }
    
    /**
     * 是否是正无穷或负无穷
     */
    bool IsInfinity() const noexcept { return ExptBits() == EXPT_BIT_MASK && !FracBits(); }
    
    /**
     * 是否是负数/-0
     */
    bool IsNegative() const noexcept { return SignBit() != 0; }

    static constexpr size_t DefaultEqEpsilon() noexcept { return DEFAULT_MAX_ULP; }

    /**
     * 是否和另一个数近似相等
     * 
     * @param rhs 进行近似比较的数值
     * @param maxULPs 近似阈值，以两个数之间最大容许的可能的取值数量表示
     */
    bool ApproxEq(Self rhs, size_t maxULPs = DEFAULT_MAX_ULP) const noexcept
    {
        if(IsNegative() != rhs.IsNegative())
            return float_ == rhs.float_;
        InternalUInt ULPsDiff = uint_ > rhs.uint_ ? uint_ - rhs.uint_ : rhs.uint_ - uint_;
        return ULPsDiff <= maxULPs;
    }

#define FP_BINARY_OPERATOR(binary_opr, assign_opr) \
    auto operator binary_opr(Self rhs) const noexcept { return Self(float_ binary_opr rhs.float_); } \
    Self &operator assign_opr(Self rhs) noexcept { float_ assign_opr rhs,float_; return *this; }

    FP_BINARY_OPERATOR(+, +=)
    FP_BINARY_OPERATOR(-, -=)
    FP_BINARY_OPERATOR(*, *=)
    FP_BINARY_OPERATOR(/, /=)

#undef FP_BINARY_OPERATOR

#define FP_COMPARE_OPERATOR(comp_opr) \
    bool operator comp_opr(Self rhs) const noexcept { return float_ comp_opr rhs.float_; }

    FP_COMPARE_OPERATOR(==)
    FP_COMPARE_OPERATOR(!=)
    FP_COMPARE_OPERATOR(<)
    FP_COMPARE_OPERATOR(>)
    FP_COMPARE_OPERATOR(<=)
    FP_COMPARE_OPERATOR(>=)

#undef FP_COMPARE_OPERATOR

    Self operator-() const noexcept { return Self(-float_); }
};

using Float  = FP<float>;
using Double = FP<double>;

} // namespace AGZ::Math
