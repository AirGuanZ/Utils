#pragma once

#include <cmath>
#include <algorithm>

#include "../Common.h"
#include "Angle.h"

AGZ_NS_BEG(AGZ::Math)

template<typename T> struct Abs_impl;
template<>           struct Abs_impl<float>  { static float Abs(float v) { return std::abs(v); } };
template<>           struct Abs_impl<double> { static double Abs(double v) { return std::fabs(v); } };
template<typename T> struct Abs_impl<Rad<T>> { static Rad<T> Abs(Rad<T> v) { return Rad<T>{ Abs_impl<T>::Abs(v.value) }; } };
template<typename T> struct Abs_impl<Deg<T>> { static Deg<T> Abs(Deg<T> v) { return Deg<T>{ Abs_impl<T>::Abs(v.value) }; } };

template<typename T> T Abs(T v) { return Abs_impl<T>::Abs(v); }

template<typename T> T      Sqrt(T);
template<>           AGZ_INLINE float  Sqrt<float>(float value)   { return std::sqrt(value); }
template<>           AGZ_INLINE double Sqrt<double>(double value) { return std::sqrt(value); }

template<typename T> T Clamp(T v, T minv, T maxv) { return (std::max)((std::min)(v, maxv), minv); }

template<typename T, typename U> bool ApproxEq(T lhs, T rhs, U epsilon) { return Abs(lhs - rhs) <= epsilon; }

AGZ_NS_END(AGZ::Math)
