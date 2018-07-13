#pragma once

#include <cstring>
#include <type_traits>

#include "../Common.h"
#include "Scalar.h"
#include "Vec3.h"
#include "Vec4.h"

AGZ_NS_BEG(AGZ)
AGZ_NS_BEG(Math)

/*
    Row-major 4x4 matrix
*/
template<typename T>
class Mat4
{
public:

    using Data = T[4][4];

    Data m;

public:

    using Component = T;
    using Self = Mat4<T>;

    Mat4(Uninitialized_t) { }

    Mat4() : Mat4(Scalar::ONE<T>()) { }

    explicit Mat4(T v);

    explicit Mat4(const Data &_m);

    Mat4(T m00, T m01, T m02, T m03,
         T m10, T m11, T m12, T m13,
         T m20, T m21, T m22, T m23,
         T m30, T m31, T m32, T m33);

    static Self All(T v);

    static const Self &IDENTITY();

    bool operator==(const Self &other) const;

    bool operator!=(const Self &other) const;

    Self operator*(const Self &rhs) const;
    
    Vec4<T> operator*(const Vec4<T> &p);
};

template<typename T>
inline Vec4<T> ApplyToPoint(const Mat4<T> &m, const Vec4<T> &v);

template<typename T>
inline Vec3<T> ApplyToPoint(const Mat4<T> &m, const Vec3<T> &p);

template<typename T>
inline Vec4<T> ApplyToVector(const Mat4<T> &m, const Vec4<T> &v);

template<typename T>
inline Vec4<T> ApplyToVector(const Mat4<T> &m, const Vec3<T> &v);

template<typename T>
inline Mat4<T> Transpose(const Mat4<T> &m);

template<typename T>
inline Mat4<T> Inverse(const Mat4<T> &m);

AGZ_NS_END(Math)
AGZ_NS_END(AGZ)

#include "Mat4.inl"
