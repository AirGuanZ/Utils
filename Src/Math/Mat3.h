#pragma once

/**
 * @file Math/Mat3.h
 * @brief 功能非常受限的3x3数值矩阵类
 */

#include <type_traits>

#include "../Misc/Common.h"
#include "Angle.h"
#include "Scalar.h"
#include "Vec3.h"

namespace AGZ::Math {

/**
 * @brief 按行存储的3x3矩阵
 */
template<typename T>
class Mat3
{
public:

    using Data = T[3][3]; ///< 所存储的数据
    using Component = T;  ///< 元素类型
    using Self = Mat3<T>; ///< 自身类型

    Data m;

    /** 初始化为单位矩阵 */
    Mat3() noexcept: Mat3(T(1)) { }

    /** 不初始化任何元素 */
    explicit Mat3(Uninitialized_t) noexcept { }

    /** 初始化为对角阵，对角元素值为v */
    explicit Mat3(T v) noexcept;

    /** 从3x3数组中取得初始化数据 */
    explicit Mat3(const Data &_m) noexcept;

    /** 逐个指定每个元素的值，mij代表第i行第j列 */
    Mat3(T m00, T m01, T m02,
         T m10, T m11, T m12,
         T m20, T m21, T m22) noexcept;

    /** 将所有元素设置为v */
    static Self All(T v) noexcept;

    /** 用三个列向量构造矩阵 */
    static Self FromCols(const Vec3<T> &col0,
                         const Vec3<T> &col1,
                         const Vec3<T> &col2) noexcept;

    /** 返回一个单位矩阵 */
    static const Self &IDENTITY() noexcept;

    /** 矩阵-矩阵乘法 */
    Self operator*(const Self &rhs) const noexcept;

    /** 矩阵-向量乘法 */
    Vec3<T> operator*(const Vec3<T> &rhs) const noexcept;

    /**
     * 构造绕指定轴的旋转矩阵
     * 
     * @param _axis 旋转轴
     * @param angle 旋转角。为Deg/Rad时会自动进行单位转换，为float/double时单位为弧度
     */
    template<typename U>
    static Self Rotate(const Vec3<T> &_axis, U angle) noexcept;

    /** 构造绕X轴的旋转矩阵 */
    template<typename U>
    static Self RotateX(U angle) noexcept;

    /** 构造绕Y轴的旋转矩阵 */
    template<typename U>
    static Self RotateY(U angle) noexcept;

    /** 构造绕Z轴的旋转矩阵 */
    template<typename U>
    static Self RotateZ(U angle) noexcept;

    /** 求行列式的值 */
    T Determinant() const noexcept;

    Self Transpose() const noexcept;

    /** 取得指定列 */
    Vec3<T> GetCol(size_t colIdx) const noexcept;
};

template<typename T>
Mat3<T>::Mat3(T v) noexcept
{
    m[0][1] = m[0][2] =
    m[1][0] = m[1][2] =
    m[2][0] = m[2][1] = T(0);
    m[0][0] = m[1][1] = m[2][2] = v;
}

template<typename T>
Mat3<T>::Mat3(const Data &_m) noexcept
{
    static_assert(std::is_trivially_copyable_v<Component>);
    std::memcpy(m, _m, sizeof(m));
}

template<typename T>
Mat3<T>::Mat3(T m00, T m01, T m02,
              T m10, T m11, T m12,
              T m20, T m21, T m22) noexcept
{
    m[0][0] = m00; m[0][1] = m01; m[0][2] = m02;
    m[1][0] = m10; m[1][1] = m11; m[1][2] = m12;
    m[2][0] = m20; m[2][1] = m21; m[2][2] = m22;
}

template<typename T>
typename Mat3<T>::Self Mat3<T>::All(T v) noexcept
{
    Self ret(UNINITIALIZED);
    ret.m[0][0] = ret.m[0][1] = ret.m[0][2] =
    ret.m[1][0] = ret.m[1][1] = ret.m[1][2] =
    ret.m[2][0] = ret.m[2][1] = ret.m[2][2] = v;
    return ret;
}

template<typename T>
typename Mat3<T>::Self Mat3<T>::FromCols(const Vec3<T> &col0,
                                         const Vec3<T> &col1,
                                         const Vec3<T> &col2) noexcept
{
    Self ret(col0.x, col1.x, col2.x,
             col0.y, col1.y, col2.y,
             col0.z, col1.z, col2.z);
    return ret;
}

template<typename T>
const typename Mat3<T>::Self &Mat3<T>::IDENTITY() noexcept
{
    static const Self ret(T(1));
    return ret;
}

template<typename T>
typename Mat3<T>::Self Mat3<T>::operator*(const Mat3<T> &rhs) const noexcept
{
    Self ret(UNINITIALIZED);
    for(int r = 0; r < 3; ++r)
    {
        for(int c = 0; c < 3; ++c)
        {
            ret.m[r][c] = m[r][0] * rhs.m[0][c]
                        + m[r][1] * rhs.m[1][c]
                        + m[r][2] * rhs.m[2][c];
        }
    }
    return ret;
}

template<typename T>
Vec3<T> Mat3<T>::operator*(const Vec3<T> &rhs) const noexcept
{
    return Vec3<T>(m[0][0] * rhs.x + m[0][1] * rhs.y + m[0][2] * rhs.z,
                   m[1][0] * rhs.x + m[1][1] * rhs.y + m[1][2] * rhs.z,
                   m[2][0] * rhs.x + m[2][1] * rhs.y + m[2][2] * rhs.z);
}

template<typename T>
template<typename U>
typename Mat3<T>::Self Mat3<T>::Rotate(const Vec3<T> &_axis, U angle) noexcept
{
    T m[3][3];
    Vec3<T> axis = Normalize(_axis);
    auto sinv = Sin(angle), cosv = Cos(angle);

    constexpr T I = T(1), O = T(0);

    m[0][0] = axis.x * axis.x + (I - axis.x * axis.x) * cosv;
    m[0][1] = axis.x * axis.y * (I - cosv) - axis.z * sinv;
    m[0][2] = axis.x * axis.z * (I - cosv) + axis.y * sinv;

    m[1][0] = axis.x * axis.y * (I - cosv) + axis.z * sinv;
    m[1][1] = axis.y * axis.y + (I - axis.y * axis.y) * cosv;
    m[1][2] = axis.y * axis.z * (I - cosv) - axis.x * sinv;

    m[2][0] = axis.x * axis.z * (I - cosv) - axis.y * sinv;
    m[2][1] = axis.y * axis.z * (I - cosv) + axis.x * sinv;
    m[2][2] = axis.z * axis.z + (I - axis.z * axis.z) * cosv;

    return Mat3<T>(m);
}

template<typename T>
template<typename U>
typename Mat3<T>::Self Mat3<T>::RotateX(U angle) noexcept
{
    constexpr T I = T(1), O = T(0);
    const auto S = Sin(angle), C = Cos(angle);
    return Mat3<T>(I, O, O,
                   O, C, -S,
                   O, S, C);
}

template <typename T>
template <typename U>
typename Mat3<T>::Self Mat3<T>::RotateY(U angle) noexcept
{
    constexpr T I = T(1), O = T(0);
    const auto S = Sin(angle), C = Cos(angle);
    return Mat3<T>(C, O, S,
                   O, I, O,
                   -S, O, C);
}

template <typename T>
template <typename U>
typename Mat3<T>::Self Mat3<T>::RotateZ(U angle) noexcept
{
    constexpr T I = T(1), O = T(0);
    const auto S = Sin(angle), C = Cos(angle);
    return Mat3<T>(C, -S, O,
                   S, C, O,
                   O, O, I);
}

template<typename T>
T Mat3<T>::Determinant() const noexcept
{
    Vec3<T> a(m[0][0], m[1][0], m[2][0]);
    Vec3<T> b(m[0][1], m[1][1], m[2][1]);
    Vec3<T> c(m[0][2], m[1][2], m[2][2]);
    return Dot(Cross(c, a), b);
}

template<typename T>
typename Mat3<T>::Self Mat3<T>::Transpose() const noexcept
{
    return Mat3<T>(m[0][0], m[1][0], m[2][0],
                   m[0][1], m[1][1], m[2][1],
                   m[0][2], m[1][2], m[2][2]);
}

template<typename T>
Vec3<T> Mat3<T>::GetCol(size_t colIdx) const noexcept
{
    AGZ_ASSERT(colIdx < 3);
    return Vec3<T>(m[0][colIdx],
                   m[1][colIdx],
                   m[2][colIdx]);
}

/** 以float为元素的3x3矩阵 */
using Mat3f = Mat3<float>;

/** 以double为元素的3x3矩阵 */
using Mat3d = Mat3<double>;

} // namespace AGZ::Math
