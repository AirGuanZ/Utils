#pragma once

#include <cmath>
#include <algorithm>

#include "../Common.h"
#include "Angle.h"

AGZ_NS_BEG(AGZ)
AGZ_NS_BEG(Math)

template<typename T> struct Abs_impl;
template<>           struct Abs_impl<float>  { static constexpr float Abs(float v) { return std::fabsf(v); } };
template<>           struct Abs_impl<double> { static constexpr double Abs(double v) { return std::fabs(v); } };
template<typename T> struct Abs_impl<Rad<T>> { static AGZ_FORCE_INLINE Rad<T> Abs(Rad<T> v) { return Rad<T>{ Abs_impl<T>::Abs(v.value) }; } };
template<typename T> struct Abs_impl<Deg<T>> { static AGZ_FORCE_INLINE Deg<T> Abs(Deg<T> v) { return Deg<T>{ Abs_impl<T>::Abs(v.value) }; } };

template<typename T> AGZ_FORCE_INLINE T Abs(T v) { return Abs_impl<T>::Abs(v); }

template<typename T> AGZ_FORCE_INLINE T      Sqrt(T);
template<>           AGZ_FORCE_INLINE float  Sqrt<float>(float value)   { return std::sqrtf(value); }
template<>           AGZ_FORCE_INLINE double Sqrt<double>(double value) { return std::sqrt(value); }

template<typename T> AGZ_FORCE_INLINE T Clamp(T v, T minv, T maxv) { return (std::max)((std::min)(v, maxv), minv); }

template<typename T> AGZ_FORCE_INLINE bool ApproxEq(T lhs, T rhs, T epsilon) { return Abs(lhs - rhs) <= epsilon; }

AGZ_NS_END(Math)
AGZ_NS_END(AGZ)
