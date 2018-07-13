#pragma once

#include <cmath>
#include <algorithm>

#include "../Common.h"
#include "Angle.h"

AGZ_NS_BEG(AGZ)
AGZ_NS_BEG(Math)

template<typename T> struct PI_impl;
template<>           struct PI_impl<float>  { static constexpr float  PI() { return 3.141592653589793238462643383f; } };
template<>           struct PI_impl<double> { static constexpr double PI() { return 3.141592653589793238462643383; } };
template<typename T> struct PI_impl<Rad<T>> { static AGZ_FORCE_INLINE Rad<T> PI() { return Rad<T>{ PI_impl<T>::PI() }; } };
template<typename T> struct PI_impl<Deg<T>> { static AGZ_FORCE_INLINE Deg<T> PI() { return Deg<T>{ T(180.0) }; } };

template<typename T> struct Abs_impl;
template<>           struct Abs_impl<float>  { static constexpr float Abs(float v) { return std::fabsf(v); } };
template<>           struct Abs_impl<double> { static constexpr double Abs(double v) { return std::fabs(v); } };
template<typename T> struct Abs_impl<Rad<T>> { static AGZ_FORCE_INLINE Rad<T> Abs(Rad<T> v) { return Rad<T>{ Abs_impl<T>::Abs(v.value) }; } };
template<typename T> struct Abs_impl<Deg<T>> { static AGZ_FORCE_INLINE Deg<T> Abs(Deg<T> v) { return Deg<T>{ Abs_impl<T>::Abs(v.value) }; } };

template<typename T> AGZ_FORCE_INLINE auto Deg2Rad(Deg<T> deg) { return Rad<T> { deg.value * (PI_impl<T>::PI() / T(180.0)) }; }
template<typename T> AGZ_FORCE_INLINE auto Rad2Deg(Rad<T> rad) { return Deg<T> { rad.value * (T(180.0) / PI_impl<T>::PI()) }; }

template<typename T> AGZ_FORCE_INLINE auto AsDeg(Deg<T> deg) { return deg; }
template<typename T> AGZ_FORCE_INLINE auto AsDeg(Rad<T> rad) { return Rad2Deg<T>(rad); }

template<typename T> AGZ_FORCE_INLINE auto AsRad(Deg<T> deg) { return Deg2Rad<T>(deg); }
template<typename T> AGZ_FORCE_INLINE auto AsRad(Rad<T> rad) { return rad; }

template<typename T> T Sin_raw_impl(T);
template<> float  Sin_raw_impl<float> (float rad)  { return std::sinf(rad); }
template<> double Sin_raw_impl<double>(double rad) { return std::sin(rad); }

template<typename T> struct Sin_impl;
template<typename T> struct Sin_impl<Rad<T>> { static AGZ_FORCE_INLINE auto Sin(Rad<T> rad) { return Sin_raw_impl<T>(rad.value); } };
template<typename T> struct Sin_impl<Deg<T>> { static AGZ_FORCE_INLINE auto Sin(Deg<T> deg) { return Sin_raw_impl<T>(deg.value * (PI_impl<T>::PI() / T(180.0))); } };

template<typename T> T Cos_raw_impl(T);
template<> float  Cos_raw_impl<float> (float rad)  { return std::cosf(rad); }
template<> double Cos_raw_impl<double>(double rad) { return std::cos(rad); }

template<typename T> struct Cos_impl;
template<typename T> struct Cos_impl<Rad<T>> { static AGZ_FORCE_INLINE auto Cos(Rad<T> rad) { return Cos_raw_impl<T>(rad.value); } };
template<typename T> struct Cos_impl<Deg<T>> { static AGZ_FORCE_INLINE auto Cos(Deg<T> deg) { return Cos_raw_impl<T>(deg.value * (PI_impl<T>::PI() / T(180.0))); } };

template<typename T> AGZ_FORCE_INLINE T PI() { return PI_impl<T>::PI(); };

template<typename T> AGZ_FORCE_INLINE T Abs(T v) { return Abs_impl<T>::Abs(v); }

template<typename T> AGZ_FORCE_INLINE auto Sin(T angle) { return Sin_impl<T>::Sin(angle); }

template<typename T> AGZ_FORCE_INLINE auto Cos(T angle) { return Cos_impl<T>::Cos(angle); }

template<typename CT> AGZ_FORCE_INLINE constexpr CT     ZERO();
template<>            AGZ_FORCE_INLINE constexpr int    ZERO<int>()    { return 0; }
template<>            AGZ_FORCE_INLINE constexpr float  ZERO<float>()  { return 0.0f; }
template<>            AGZ_FORCE_INLINE constexpr double ZERO<double>() { return 0.0; }

template<typename CT> AGZ_FORCE_INLINE constexpr CT     ONE();
template<>            AGZ_FORCE_INLINE constexpr int    ONE<int>()    { return 1; }
template<>            AGZ_FORCE_INLINE constexpr float  ONE<float>()  { return 1.0f; }
template<>            AGZ_FORCE_INLINE constexpr double ONE<double>() { return 1.0; }

template<typename T> AGZ_FORCE_INLINE T      Sqrt(T);
template<>           AGZ_FORCE_INLINE float  Sqrt<float>(float value)   { return std::sqrtf(value); }
template<>           AGZ_FORCE_INLINE double Sqrt<double>(double value) { return std::sqrt(value); }

template<typename T> AGZ_FORCE_INLINE T Clamp(T v, T minv, T maxv) { return (std::max)((std::min)(v, maxv), minv); }

template<typename T> AGZ_FORCE_INLINE bool ApproxEq(T lhs, T rhs, T epsilon) { return Abs(lhs - rhs) <= epsilon; }

AGZ_NS_END(Math)
AGZ_NS_END(AGZ)
