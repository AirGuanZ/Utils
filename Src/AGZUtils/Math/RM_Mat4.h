#pragma once

/**
 * @file Math/Mat4.h
 * @brief 4x4数值矩阵类，主要用于图形学中的几何变换
 */

#include <algorithm>
#include <cstring>
#include <type_traits>

#include "../Misc/Common.h"
#include "Scalar.h"
#include "Vec3.h"
#include "Vec4.h"

namespace AGZ::Math {

/**
 * @brief 按行存储的4x4矩阵
 */
template<typename T>
class RM_Mat4
{
public:

    using Data      = T[4][4];    ///< 所存储的数据
    using Component = T;          ///< 元素类型
    using Self      = RM_Mat4<T>; ///< 自身类型

    Data m;

    /** 初始化为单位矩阵 */
    RM_Mat4() noexcept: RM_Mat4(T(1)) { }

    /** 不初始化任何元素 */
    explicit RM_Mat4(Uninitialized_t) noexcept { }

    /** 初始化为对角阵，对角元素值为v */
    explicit RM_Mat4(T v) noexcept;

    /** 从4x4数组中取得初始化数据 */
    explicit RM_Mat4(const Data &_m) noexcept;

    /** 逐个指定每个元素的值，mij代表第i行第j列 */
    RM_Mat4(T m00, T m01, T m02, T m03,
            T m10, T m11, T m12, T m13,
            T m20, T m21, T m22, T m23,
            T m30, T m31, T m32, T m33) noexcept;

    /** 将所有元素设置为v */
    static Self All(T v) noexcept;

    /** 返回一个单位矩阵 */
    static const Self &IDENTITY() noexcept;

    bool operator==(const Self &other) const noexcept;

    bool operator!=(const Self &other) const noexcept;

    /** 矩阵-矩阵乘法 */
    Self operator*(const Self &rhs) const noexcept;

    /** 矩阵-向量乘法 */
    Vec4<T> operator*(const Vec4<T> &p) const noexcept;

    /**
     * 构造平移矩阵
     * 
     * @param v 平移向量
     */
    static Self Translate(const Vec3<T> &v) noexcept;

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

    /** 构造缩放矩阵 */
    static Self Scale(const Vec3<T> &s) noexcept;

    /**
     * 构造透视投影矩阵
     * 
     * @param fovY 竖直方向视野角
     * @param ratio 视野宽度/视野高度
     * @param _near 近截面和视点的距离
     * @param _far 远截面和视点的距离
     */
    template<typename U>
    static Self Perspective(U fovY, T ratio, T _near, T _far) noexcept;

    /**
     * 构造视点矩阵
     * 
     * @param src 眼睛位置
     * @param dst 目标点位置
     * @param up 用来确定视野侧向倾斜角度的up向量
     */
    static Self LookAt(const Vec3<T> &src, const Vec3<T> &dst, const Vec3<T> &up) noexcept;

    /** 将矩阵变换作用在点上并进行齐次坐标归一化 */
    Vec4<T> ApplyToPoint(const Vec4<T> &p) const noexcept;
    //! @copydoc Mat4<T>::ApplyToPoint(const Vec4<T> &) const
    Vec3<T> ApplyToPoint(const Vec3<T> &p) const noexcept;

    /** 将矩阵变换作用在方向上 */
    Vec4<T> ApplyToVector(const Vec4<T> &p) const noexcept;
    //! @copydoc Mat4<T>::ApplyToVector(const Vec4<T> &) const
    Vec3<T> ApplyToVector(const Vec3<T> &p) const noexcept;

    /** 将逆变换作用在法线上 */
    Vec3<T> ApplyInverseToNormal(const Vec3<T> &n) const noexcept;

    /** 转置矩阵 */
    Self Transpose() const noexcept;

    /**
     * 逆矩阵
     * 
     * @warning 若原矩阵不可逆，会造成UB
     */
    Self Inverse() const noexcept;
};

/** 求给定矩阵的转置 */
template<typename T>
RM_Mat4<T> Transpose(const RM_Mat4<T> &m) noexcept;

/** 求给定矩阵的逆矩阵。若原矩阵不可逆，则UB。 */
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
RM_Mat4<T> Inverse(const RM_Mat4<T> &m) noexcept;

using RM_Mat4f = RM_Mat4<float>;
using RM_Mat4d = RM_Mat4<double>;

//===================================================== Implementations =====================================================

template<typename T>
RM_Mat4<T>::RM_Mat4(T v) noexcept
{
    m[0][1] = m[0][2] = m[0][3] =
    m[1][0] = m[1][2] = m[1][3] =
    m[2][0] = m[2][1] = m[2][3] =
    m[3][0] = m[3][1] = m[3][2] = T(0);
    m[0][0] = m[1][1] = m[2][2] = m[3][3] = v;
}

template<typename T>
RM_Mat4<T>::RM_Mat4(const Data &_m) noexcept
{
    static_assert(std::is_trivially_copyable_v<Component>);
    std::memcpy(m, _m, sizeof(m));
}

template<typename T>
RM_Mat4<T>::RM_Mat4(T m00, T m01, T m02, T m03,
                    T m10, T m11, T m12, T m13,
                    T m20, T m21, T m22, T m23,
                    T m30, T m31, T m32, T m33) noexcept
{
    m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
    m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
    m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
    m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
}

template<typename T>
RM_Mat4<T> RM_Mat4<T>::All(T v) noexcept
{
    Self ret(UNINITIALIZED);
    ret.m[0][0] = ret.m[0][1] = ret.m[0][2] = ret.m[0][3] =
    ret.m[1][0] = ret.m[1][1] = ret.m[1][2] = ret.m[1][3] =
    ret.m[2][0] = ret.m[2][1] = ret.m[2][2] = ret.m[2][3] =
    ret.m[3][0] = ret.m[3][1] = ret.m[3][2] = ret.m[3][3] = v;
    return ret;
}

template<typename T>
const RM_Mat4<T> &RM_Mat4<T>::IDENTITY() noexcept
{
    static const Self ret(T(1));
    return ret;
}

template<typename T>
bool RM_Mat4<T>::operator==(const Self &other) const noexcept
{
    for(int c = 0; c < 4; ++c)
    {
        for(int r = 0; r < 4; ++r)
        {
            if(m[c][r] != other[c][r])
                return false;
        }
    }
    return true;
}

template<typename T>
bool RM_Mat4<T>::operator!=(const Self &other) const noexcept
{
    for(int c = 0; c < 4; ++c)
    {
        for(int r = 0; r < 4; ++r)
        {
            if(m[c][r] != other[c][r])
                return true;
        }
    }
    return false;
}

template<typename T>
typename RM_Mat4<T>::Self RM_Mat4<T>::operator*(const Self &rhs) const noexcept
{
    Self ret(UNINITIALIZED);
    for(int r = 0; r < 4; ++r)
    {
        for(int c = 0; c < 4; ++c)
        {
            ret.m[r][c] = m[r][0] * rhs.m[0][c]
                        + m[r][1] * rhs.m[1][c]
                        + m[r][2] * rhs.m[2][c]
                        + m[r][3] * rhs.m[3][c];
        }
    }
    return ret;
}

template<typename T>
Vec4<T> RM_Mat4<T>::operator*(const Vec4<T> &p) const noexcept
{
    return Vec4<T>(m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z + m[0][3] * p.w,
                   m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z + m[1][3] * p.w,
                   m[2][0] * p.x + m[2][1] * p.y + m[2][2] * p.z + m[2][3] * p.w,
                   m[3][0] * p.x + m[3][1] * p.y + m[3][2] * p.z + m[3][3] * p.w);
}

template<typename T>
typename RM_Mat4<T>::Self RM_Mat4<T>::Translate(const Vec3<T> &v) noexcept
{
    constexpr T I = T(1), O = T(0);
    return Self(I, O, O, v.x,
                O, I, O, v.y,
                O, O, I, v.z,
                O, O, O, I);
}

template<typename T>
template<typename U>
typename RM_Mat4<T>::Self RM_Mat4<T>::Rotate(const Vec3<T> &_axis, U angle) noexcept
{
    T m[4][4];
    Vec3<T> axis = Normalize(_axis);
    auto sinv = Sin(angle), cosv = Cos(angle);

    constexpr T I = T(1), O = T(0);

    m[0][0] = axis.x * axis.x + (I - axis.x * axis.x) * cosv;
    m[0][1] = axis.x * axis.y * (I - cosv) - axis.z * sinv;
    m[0][2] = axis.x * axis.z * (I - cosv) + axis.y * sinv;
    m[0][3] = O;

    m[1][0] = axis.x * axis.y * (I - cosv) + axis.z * sinv;
    m[1][1] = axis.y * axis.y + (I - axis.y * axis.y) * cosv;
    m[1][2] = axis.y * axis.z * (I - cosv) - axis.x * sinv;
    m[1][3] = O;

    m[2][0] = axis.x * axis.z * (I - cosv) - axis.y * sinv;
    m[2][1] = axis.y * axis.z * (I - cosv) + axis.x * sinv;
    m[2][2] = axis.z * axis.z + (I - axis.z * axis.z) * cosv;
    m[2][3] = O;

    m[3][0] = O;
    m[3][1] = O;
    m[3][2] = O;
    m[3][3] = I;

    return Self(m);
}

template<typename T>
template<typename U>
typename RM_Mat4<T>::Self RM_Mat4<T>::RotateX(U angle) noexcept
{
    constexpr T I = T(1), O = T(0);
    const auto S = Sin(angle), C = Cos(angle);
    return Self(I, O, O, O,
                O, C, -S, O,
                O, S, C, O,
                O, O, O, I);
}

template <typename T>
template <typename U>
typename RM_Mat4<T>::Self RM_Mat4<T>::RotateY(U angle) noexcept
{
    constexpr T I = T(1), O = T(0);
    const auto S = Sin(angle), C = Cos(angle);
    return Self(C, O, S, O,
                O, I, O, O,
                -S, O, C, O,
                O, O, O, I);
}

template <typename T>
template <typename U>
typename RM_Mat4<T>::Self RM_Mat4<T>::RotateZ(U angle) noexcept
{
    constexpr T I = T(1), O = T(0);
    const auto S = Sin(angle), C = Cos(angle);
    return Self(C, -S, O, O,
                S, C, O, O,
                O, O, I, O,
                O, O, O, I);
}

template<typename T>
typename RM_Mat4<T>::Self RM_Mat4<T>::Scale(const Vec3<T> &s) noexcept
{
    constexpr T I = T(1), O = T(0);
    return Self(s.x, O, O, O,
                O, s.y, O, O,
                O, O, s.z, O,
                O, O, O, I);
}

template<typename T>
template<typename U>
typename RM_Mat4<T>::Self RM_Mat4<T>::Perspective(U fovY, T ratio, T _near, T _far) noexcept
{
    T invDis = T(1) / (_far - _near);
    constexpr T I = T(1), O = T(0);
    auto cot = Cot(T(0.5) * fovY);
    return Self(cot/ratio, O,   O,             O,
                O,         cot, O,             O,
                O,         O,   _far * invDis, -_far * _near * invDis,
                O,         O,   I,             O);
}

template<typename T>
typename RM_Mat4<T>::Self RM_Mat4<T>::LookAt(const Vec3<T>& src, const Vec3<T>& dst, const Vec3<T>& up) noexcept
{
    constexpr T I = T(1), O = T(0);
    auto D = Normalize(dst - src);
    auto R = Normalize(Cross(up, D));
    auto U = Cross(D, R);
    return Self(R.x, U.x, D.x, src.x,
                R.y, U.y, D.y, src.y,
                R.z, U.z, D.z, src.z,
                O, O, O, I).Inverse();
}

template<typename T>
Vec4<T> RM_Mat4<T>::ApplyToPoint(const Vec4<T> &v) const noexcept
{
    return *this * v;
}

template<typename T>
Vec3<T> RM_Mat4<T>::ApplyToPoint(const Vec3<T> &p) const noexcept
{
    Vec4<T> ret = *this * Vec4<T>(p.x, p.y, p.z, T(1.0));
    T dw = T(1) / ret.w;
    return Vec3<T>(dw * ret.x, dw * ret.y, dw * ret.z);
}

template<typename T>
Vec4<T> RM_Mat4<T>::ApplyToVector(const Vec4<T> &v) const noexcept
{
    return *this * v;
}

template<typename T>
Vec3<T> RM_Mat4<T>::ApplyToVector(const Vec3<T> &v) const noexcept
{
    return Vec3<T>(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
                   m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
                   m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z);
}

template<typename T>
Vec3<T> RM_Mat4<T>::ApplyInverseToNormal(const Vec3<T> &n) const noexcept
{
    return Vec3<T>(m[0][0] * n.x + m[1][0] * n.y + m[2][0] * n.z,
                   m[0][1] * n.x + m[1][1] * n.y + m[2][1] * n.z, 
                   m[0][2] * n.x + m[1][2] * n.y + m[2][2] * n.z);
}

template<typename T>
RM_Mat4<T> Transpose(const RM_Mat4<T> &m) noexcept
{
    return RM_Mat4<T>(m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0],
                      m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1],
                      m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2],
                      m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]);
}

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>>
RM_Mat4<T> Inverse(const RM_Mat4<T> &_m) noexcept
{
    int indxc[4], indxr[4], ipiv[4] = { 0 };
    T m[4][4];
    std::memcpy(m, _m.m, 4 * 4 * sizeof(T));
    for(int i = 0; i < 4; i++)
    {
        T largest = T(0);
        int irow = -1, icol = -1;
        for(int j = 0; j < 4; j++)
        {
            if(ipiv[j] != 1)
            {
                for(int k = 0; k < 4; k++)
                {
                    if(ipiv[k] == 0)
                    {
                        if(Abs(m[j][k]) >= largest)
                        {
                            irow = j, icol = k;
                            largest = Abs(m[j][k]);
                        }
                    }
                    AGZ_ASSERT(ipiv[k] <= 1);
                }
            }
        }

        ++ipiv[icol];
        if(irow != icol)
        {
            std::swap(m[irow][0], m[icol][0]);
            std::swap(m[irow][1], m[icol][1]);
            std::swap(m[irow][2], m[icol][2]);
            std::swap(m[irow][3], m[icol][3]);
        }
        indxr[i] = irow, indxc[i] = icol;
        AGZ_ASSERT(m[icol][icol] != T(0.));

        T pivinv = T(1.0) / m[icol][icol];
        m[icol][icol] = T(1.0);
        m[icol][0] *= pivinv;
        m[icol][1] *= pivinv;
        m[icol][2] *= pivinv;
        m[icol][3] *= pivinv;

        for(int j = 0; j < 4; j++)
        {
            if(j != icol)
            {
                T save = m[j][icol];
                m[j][icol] = 0;
                m[j][0] -= m[icol][0] * save;
                m[j][1] -= m[icol][1] * save;
                m[j][2] -= m[icol][2] * save;
                m[j][3] -= m[icol][3] * save;
            }
        }
    }

    for(int j = 3; j >= 0; j--)
    {
        if(indxr[j] != indxc[j])
        {
            std::swap(m[0][indxr[j]], m[0][indxc[j]]);
            std::swap(m[1][indxr[j]], m[1][indxc[j]]);
            std::swap(m[2][indxr[j]], m[2][indxc[j]]);
            std::swap(m[3][indxr[j]], m[3][indxc[j]]);
        }
    }

    return RM_Mat4<T>(m);
}

template<typename T>
RM_Mat4<T> RM_Mat4<T>::Transpose() const noexcept
{
    return ::AGZ::Math::Transpose(*this);
}

template<typename T>
RM_Mat4<T> RM_Mat4<T>::Inverse() const noexcept
{
    return ::AGZ::Math::Inverse(*this);
}

} // namespace AGZ::Math
