#pragma once

#include <cmath>

#include "../Common.h"
#include "Angle.h"

AGZ_NS_BEG(AGZ)
AGZ_NS_BEG(Math)

template<typename T> struct PI_impl;
template<>           struct PI_impl<float>  { static constexpr float  PI() { return 3.141592653589793238462643383f; } };
template<>           struct PI_impl<double> { static constexpr double PI() { return 3.141592653589793238462643383; } };
template<typename T> struct PI_impl<Rad<T>> { static AGZ_FORCE_INLINE Rad<T> PI() { return Rad<T>{ PI_impl<T>::PI() }; } };
template<typename T> struct PI_impl<Deg<T>> { static AGZ_FORCE_INLINE Deg<T> PI() { return Deg<T>{ T(180.0) }; } };

template<typename T> AGZ_FORCE_INLINE auto Deg2Rad(Deg<T> deg) { return Rad<T> { deg.value * (PI_impl<T>::PI() / T(180.0)) }; }
template<typename T> AGZ_FORCE_INLINE auto Rad2Deg(Rad<T> rad) { return Deg<T> { rad.value * (T(180.0) / PI_impl<T>::PI()) }; }

template<typename T> AGZ_FORCE_INLINE auto AsDeg(Deg<T> deg) { return deg; }
template<typename T> AGZ_FORCE_INLINE auto AsDeg(Rad<T> rad) { return Rad2Deg<T>(rad); }

template<typename T> AGZ_FORCE_INLINE auto AsRad(Deg<T> deg) { return Deg2Rad<T>(deg); }
template<typename T> AGZ_FORCE_INLINE auto AsRad(Rad<T> rad) { return rad; }

template<typename T> AGZ_FORCE_INLINE T Sin_rawimpl(T);
template<>           AGZ_FORCE_INLINE float  Sin_rawimpl<float>(float rad) { return std::sinf(rad); }
template<>           AGZ_FORCE_INLINE double Sin_rawimpl<double>(double rad) { return std::sin(rad); }

template<typename T> struct Sin_impl;
template<typename T> struct Sin_impl<Rad<T>> { static AGZ_FORCE_INLINE auto Sin(Rad<T> rad) { return Sin_rawimpl<T>(rad.value); } };
template<typename T> struct Sin_impl<Deg<T>> { static AGZ_FORCE_INLINE auto Sin(Deg<T> deg) { return Sin_rawimpl<T>(deg.value * (PI_impl<T>::PI() / T(180.0))); } };

template<typename T> AGZ_FORCE_INLINE T Cos_rawimpl(T);
template<>           AGZ_FORCE_INLINE float  Cos_rawimpl<float>(float rad) { return std::cosf(rad); }
template<>           AGZ_FORCE_INLINE double Cos_rawimpl<double>(double rad) { return std::cos(rad); }

template<typename T> struct Cos_impl;
template<typename T> struct Cos_impl<Rad<T>> { static AGZ_FORCE_INLINE auto Cos(Rad<T> rad) { return Cos_rawimpl<T>(rad.value); } };
template<typename T> struct Cos_impl<Deg<T>> { static AGZ_FORCE_INLINE auto Cos(Deg<T> deg) { return Cos_rawimpl<T>(deg.value * (PI_impl<T>::PI() / T(180.0))); } };

template<typename T> AGZ_FORCE_INLINE T    PI() { return PI_impl<T>::PI(); }
template<typename T> AGZ_FORCE_INLINE auto Sin(T angle) { return Sin_impl<T>::Sin(angle); }
template<typename T> AGZ_FORCE_INLINE auto Cos(T angle) { return Cos_impl<T>::Cos(angle); }
template<typename T> AGZ_FORCE_INLINE auto Tan(T angle) { return Sin<T>(angle) / Cos<T>(angle); }
template<typename T> AGZ_FORCE_INLINE auto Cot(T angle) { return Cos<T>(angle) / Sin<T>(angle); }

AGZ_NS_END(Math)
AGZ_NS_END(AGZ)
