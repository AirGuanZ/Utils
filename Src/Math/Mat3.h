#pragma once

#include <type_traits>

#include "../Misc/Common.h"
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
    Mat3() : Mat3(T(1)) { }

    /** 不初始化任何元素 */
    explicit Mat3(Uninitialized_t) { }

    /** 初始化为对角阵，对角元素值为v */
    explicit Mat3(T v)
    {
        m[0][1] = m[0][2] =
        m[1][0] = m[1][2] =
        m[2][0] = m[2][1] = T(0);
        m[0][0] = m[1][1] = m[2][2] = v;
    }

    /** 从3x3数组中取得初始化数据 */
    explicit Mat3(const Data &_m)
    {
        static_assert(std::is_trivially_copyable_v<Component>);
        std::memcpy(m, _m, sizeof(m));
    }

    /** 逐个指定每个元素的值，mij代表第i行第j列 */
    Mat3(T m00, T m01, T m02,
         T m10, T m11, T m12,
         T m20, T m21, T m22)
    {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22;
    }

    /** 将所有元素设置为v */
    static Self All(T v)
    {
        Self ret(UNINITIALIZED);
        ret.m[0][0] = ret.m[0][1] = ret.m[0][2] =
        ret.m[1][0] = ret.m[1][1] = ret.m[1][2] =
        ret.m[2][0] = ret.m[2][1] = ret.m[2][2] = v;
        return ret;
    }

    /** 用三个列向量构造矩阵 */
    static Self FromCols(const Vec3<T> &col0,
                         const Vec3<T> &col1,
                         const Vec3<T> &col2)
    {
        Self ret(col0.x, col1.x, col2.x,
                 col0.y, col1.y, col2.y,
                 col0.z, col1.z, col2.z);
        return ret;
    }

    /** 返回一个单位矩阵 */
    static const Self &IDENTITY()
    {
        static const Self ret(T(1));
        return ret;
    }

    /** 求行列式的值 */
    auto Determinant() const
    {
        Vec3<T> a(m[0][0], m[1][0], m[2][0]);
        Vec3<T> b(m[0][1], m[1][1], m[2][1]);
        Vec3<T> c(m[0][2], m[1][2], m[2][2]);
        return Dot(Cross(c, a), b);
    }
};

/** 以float为元素的3x3矩阵 */
using Mat3f = Mat3<float>;

/** 以double为元素的3x3矩阵 */
using Mat3d = Mat3<double>;

} // namespace AGZ::Math
