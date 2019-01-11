#pragma once

#include <cmath>
#include <algorithm>
#include <limits>

#include "../Misc/Common.h"
#include "../Misc/TypeOpr.h"

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
    constexpr Deg(Rad<T> r) noexcept : value(T(180) / T(3.141592653589793238462643383) * r.value) { }
};

template<typename T>
constexpr Rad<T>::Rad(const Deg<T> &d) noexcept : value(T(3.141592653589793238462643383) / T(180) * d.value)
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

namespace Impl
{
    template<typename T> struct Abs_impl         { static T Abs(T v)           noexcept { return std::abs(v); } };
    template<typename T> struct Abs_impl<Rad<T>> { static Rad<T> Abs(Rad<T> v) noexcept { return Rad<T>{ Abs_impl<T>::Abs(v.value) }; } };
    template<typename T> struct Abs_impl<Deg<T>> { static Deg<T> Abs(Deg<T> v) noexcept { return Deg<T>{ Abs_impl<T>::Abs(v.value) }; } };

    template<typename T> struct PI_impl;
    template<>           struct PI_impl<float>  { static constexpr float  PI() noexcept { return 3.141592653589793238462643383f; } };
    template<>           struct PI_impl<double> { static constexpr double PI() noexcept { return 3.141592653589793238462643383; } };
    template<typename T> struct PI_impl<Rad<T>> { static constexpr Rad<T> PI() noexcept { return Rad<T>{ PI_impl<T>::PI() }; } };
    template<typename T> struct PI_impl<Deg<T>> { static constexpr Deg<T> PI() noexcept { return Deg<T>{ T(180.0) }; } };

    template<typename T> struct Sin_impl;
    template<>           struct Sin_impl<float>  { static auto Sin(float rad)  noexcept(noexcept(std::sin(rad)))       { return std::sin(rad); } };
    template<>           struct Sin_impl<double> { static auto Sin(double rad) noexcept(noexcept(std::sin(rad)))       { return std::sin(rad); } };
    template<typename T> struct Sin_impl<Rad<T>> { static auto Sin(Rad<T> rad) noexcept(noexcept(std::sin(rad.value))) { return std::sin(rad.value); } };
    template<typename T> struct Sin_impl<Deg<T>> { static auto Sin(Deg<T> deg) noexcept(noexcept(std::sin(deg.value))) { return std::sin(deg.value * (PI_impl<T>::PI() / T(180.0))); } };

    template<typename T> struct Cos_impl;
    template<>           struct Cos_impl<float>  { static auto Cos(float rad)  noexcept(noexcept(std::cos(rad)))       { return std::cos(rad); } };
    template<>           struct Cos_impl<double> { static auto Cos(double rad) noexcept(noexcept(std::cos(rad)))       { return std::cos(rad); } };
    template<typename T> struct Cos_impl<Rad<T>> { static auto Cos(Rad<T> rad) noexcept(noexcept(std::cos(rad.value))) { return std::cos(rad.value); } };
    template<typename T> struct Cos_impl<Deg<T>> { static auto Cos(Deg<T> deg) noexcept(noexcept(std::cos(deg.value))) { return std::cos(deg.value * (PI_impl<T>::PI() / T(180.0))); } };

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

template<typename T> constexpr auto Deg2Rad(Deg<T> deg) noexcept { return Rad<T> { deg.value * (Impl::PI_impl<T>::PI() / T(180.0)) }; }
template<typename T> constexpr auto Rad2Deg(Rad<T> rad) noexcept { return Deg<T> { rad.value * (T(180.0) / Impl::PI_impl<T>::PI()) }; }

template<typename T> constexpr auto AsDeg(Deg<T> deg) noexcept { return deg; }
template<typename T> constexpr auto AsDeg(Rad<T> rad) noexcept { return Rad2Deg<T>(rad); }

template<typename T> constexpr auto AsRad(Deg<T> deg) noexcept { return Deg2Rad<T>(deg); }
template<typename T> constexpr auto AsRad(Rad<T> rad) noexcept { return rad; }

template<typename T> constexpr auto PI     = Impl::PI_impl<T>::PI();
template<typename T> constexpr auto PIx2   = T(2) * PI<T>;
template<typename T> constexpr auto PIx4   = T(4) * PI<T>;
template<typename T> constexpr auto InvPI  = T(1) / PI<T>;
template<typename T> constexpr auto Inv2PI = T(1) / PIx2<T>;
template<typename T> constexpr auto Inv4PI = T(1) / PIx4<T>;

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

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
constexpr T OneMinusEpsilon = Impl::OneMinusEpsilonImpl<T>::Value;

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
bool IsInf(T value) noexcept { return std::isinf(value); }

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
bool IsNAN(T value) noexcept { return std::isnan(value); }

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
constexpr T Inf = std::numeric_limits<T>::infinity();

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

    ValueType float_;

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
        F ret;
        std::memcpy(static_cast<void*>(&ret), static_cast<void*>(&uint), sizeof(uint));
        return ret;
    }

    static InternalUInt Value2Bits(ValueType v) noexcept
    {
        InternalUInt uint;
        std::memcpy(static_cast<void*>(&uint), static_cast<void*>(&v), sizeof(uint));
        return uint;
    }

public:

    constexpr FP() noexcept: float_(F(0)) { }
    constexpr FP(F v) noexcept: float_(v) { }
    explicit  FP(Uninitialized_t) noexcept { }

    /**
     * 取得浮点值
     */
    ValueType Value() const noexcept { return float_; }

    /**
     * 取得无符号整数表示
     */
    InternalUInt Bits() const noexcept { return Value2Bits(float_); }

    /**
     * 取得尾数
     */
    InternalUInt ExptBits() const noexcept { return EXPT_BIT_MASK & Value2Bits(float_); }
    
    /**
     * 取得阶码
     */
    InternalUInt FracBits() const noexcept { return FRAC_BIT_MASK & Value2Bits(float_); }
    
    /**
     * 取得符号位
     */
    InternalUInt SignBit()  const noexcept { return SIGN_BIT_MASK & Value2Bits(float_); }

    operator ValueType() const noexcept { return Value(); }

    /**
     * 是否是NAN
     */
    bool IsNAN() const noexcept { return IsNAN(float_); }
    
    /**
     * 是否是正无穷或负无穷
     */
    bool IsInfinity() const noexcept { return IsInf(float_); }

    static constexpr size_t DefaultEqEpsilon() noexcept { return DEFAULT_MAX_ULP; }

    /**
     * 是否和另一个数近似相等
     * 
     * @param rhs 进行近似比较的数值
     * @param maxULPs 近似阈值，以两个数之间最大容许的可能的取值数量表示
     */
    bool ApproxEq(Self rhs, size_t maxULPs = DEFAULT_MAX_ULP) const noexcept
    {
        if((float_ < 0) != (rhs.float_ < 0))
            return float_ == rhs.float_;
        InternalUInt uint = Value2Bits(float_), ruint = Value2Bits(rhs.float_);
        InternalUInt ULPsDiff = uint > ruint ? uint - ruint : ruint - uint;
        return ULPsDiff <= maxULPs;
    }
};

using Float  = FP<float>;
using Double = FP<double>;

} // namespace AGZ::Math
