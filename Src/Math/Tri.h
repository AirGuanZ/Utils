#pragma once

#include <cmath>

#include "../Misc/Common.h"
#include "Angle.h"

AGZ_NS_BEG(AGZ::Math)

template<typename T> struct PI_impl;
template<>           struct PI_impl<float>  { static constexpr float  PI() { return 3.141592653589793238462643383f; } };
template<>           struct PI_impl<double> { static constexpr double PI() { return 3.141592653589793238462643383; } };
template<typename T> struct PI_impl<Rad<T>> { static constexpr Rad<T> PI() { return Rad<T>{ PI_impl<T>::PI() }; } };
template<typename T> struct PI_impl<Deg<T>> { static constexpr Deg<T> PI() { return Deg<T>{ T(180.0) }; } };
template<typename T> constexpr auto Deg2Rad(Deg<T> deg) { return Rad<T> { deg.value * (PI_impl<T>::PI() / T(180.0)) }; }
template<typename T> constexpr auto Rad2Deg(Rad<T> rad) { return Deg<T> { rad.value * (T(180.0) / PI_impl<T>::PI()) }; }

template<typename T> constexpr auto AsDeg(Deg<T> deg) { return deg; }
template<typename T> constexpr auto AsDeg(Rad<T> rad) { return Rad2Deg<T>(rad); }

template<typename T> constexpr auto AsRad(Deg<T> deg) { return Deg2Rad<T>(deg); }
template<typename T> constexpr auto AsRad(Rad<T> rad) { return rad; }

template<typename T> T Sin_rawimpl(T);
template<>           AGZ_INLINE float  Sin_rawimpl<float>(float rad) { return std::sin(rad); }
template<>           AGZ_INLINE double Sin_rawimpl<double>(double rad) { return std::sin(rad); }

template<typename T> struct Sin_impl;
template<>           struct Sin_impl<float>  { static auto Sin(float rad)  { return Sin_rawimpl<float>(rad); } };
template<>           struct Sin_impl<double> { static auto Sin(double rad) { return Sin_rawimpl<double>(rad); } };
template<typename T> struct Sin_impl<Rad<T>> { static auto Sin(Rad<T> rad) { return Sin_rawimpl<T>(rad.value); } };
template<typename T> struct Sin_impl<Deg<T>> { static auto Sin(Deg<T> deg) { return Sin_rawimpl<T>(deg.value * (PI_impl<T>::PI() / T(180.0))); } };

template<typename T> T Cos_rawimpl(T);
template<>           AGZ_INLINE float  Cos_rawimpl<float>(float rad) { return std::cos(rad); }
template<>           AGZ_INLINE double Cos_rawimpl<double>(double rad) { return std::cos(rad); }

template<typename T> struct Cos_impl;
template<>           struct Cos_impl<float>  { static auto Cos(float rad)  { return Cos_rawimpl<float>(rad); } };
template<>           struct Cos_impl<double> { static auto Cos(double rad) { return Cos_rawimpl<double>(rad); } };
template<typename T> struct Cos_impl<Rad<T>> { static auto Cos(Rad<T> rad) { return Cos_rawimpl<T>(rad.value); } };
template<typename T> struct Cos_impl<Deg<T>> { static auto Cos(Deg<T> deg) { return Cos_rawimpl<T>(deg.value * (PI_impl<T>::PI() / T(180.0))); } };

template<typename T> constexpr T PI() { return PI_impl<T>::PI(); }
template<typename T> auto Sin(T angle) { return Sin_impl<T>::Sin(angle); }
template<typename T> auto Cos(T angle) { return Cos_impl<T>::Cos(angle); }
template<typename T> auto Tan(T angle) { return Sin<T>(angle) / Cos<T>(angle); }
template<typename T> auto Cot(T angle) { return Cos<T>(angle) / Sin<T>(angle); }

AGZ_NS_END(AGZ::Math)
