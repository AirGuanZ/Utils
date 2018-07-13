#pragma once

#include <cmath>
#include <algorithm>
#include "../Common.h"

AGZ_NS_BEG(AGZ)
AGZ_NS_BEG(Math)
AGZ_NS_BEG(Scalar)

template<typename CT> AGZ_FORCE_INLINE constexpr CT     ZERO();
template<>            AGZ_FORCE_INLINE constexpr int    ZERO<int>() { return 0; }
template<>            AGZ_FORCE_INLINE constexpr float  ZERO<float>() { return 0.0f; }
template<>            AGZ_FORCE_INLINE constexpr double ZERO<double>() { return 0.0; }

template<typename CT> AGZ_FORCE_INLINE constexpr CT     ONE();
template<>            AGZ_FORCE_INLINE constexpr int    ONE<int>() { return 1; }
template<>            AGZ_FORCE_INLINE constexpr float  ONE<float>() { return 1.0f; }
template<>            AGZ_FORCE_INLINE constexpr double ONE<double>() { return 1.0; }

template<typename T> AGZ_FORCE_INLINE T Sqrt(T);
template<> AGZ_FORCE_INLINE float  Sqrt<float>(float value) { return std::sqrtf(value); }
template<> AGZ_FORCE_INLINE double Sqrt<double>(double value) { return std::sqrt(value); }

template<typename T> AGZ_FORCE_INLINE T Clamp(T v, T minv, T maxv) { return (std::max)((std::min)(v, maxv), minv); }

template<typename T> AGZ_FORCE_INLINE T Abs(T);
template<> AGZ_FORCE_INLINE int    Abs<int>   (int v)    { return v < 0 ? -v : v; }
template<> AGZ_FORCE_INLINE float  Abs<float> (float v)  { return std::fabsf(v); }
template<> AGZ_FORCE_INLINE double Abs<double>(double v) { return std::fabs(v); }

AGZ_NS_END(Scalar)
AGZ_NS_END(Math)
AGZ_NS_END(AGZ)
