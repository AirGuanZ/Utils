#pragma once

#include "../Misc/Common.h"
#include "Mat3.h"
#include "Tri.h"
#include "Vec3.h"

namespace AGZ::Math {

/**
 * @cond
 */

namespace SHImpl
{
    // See https://en.wikipedia.org/wiki/Table_of_spherical_harmonics
    template<typename T, int L, int M> struct SHAux { };

    template<typename T>
    T Zero([[maybe_unused]] const Vec3<T>&)
    {
        return T(0);
    }

    template<typename T> struct SHAux<T, 0, 0>
    {
        static T Eval([[maybe_unused]] const Vec3<T> &dir)
        {
            static const T COEF = T(0.5) * Sqrt(1 / PI<T>);
            return COEF;
        }
    };

    template<typename T> struct SHAux<T, 1, -1>
    {
        static T Eval(const Vec3<T> &dir)
        {
            AGZ_ASSERT(ApproxEq<T>(dir.Length(), T(1), T(1e-4)));
            static const T COEF = Sqrt(3 / (4 * PI<T>));
            return COEF * dir.y;
        }
    };

    template<typename T> struct SHAux<T, 1, 0>
    {
        static T Eval(const Vec3<T> &dir)
        {
            AGZ_ASSERT(ApproxEq<T>(dir.Length(), T(1), T(1e-4)));
            static const T COEF = Sqrt(3 / (4 * PI<T>));
            return COEF * dir.z;
        }
    };

    template<typename T> struct SHAux<T, 1, 1>
    {
        static T Eval(const Vec3<T> &dir)
        {
            AGZ_ASSERT(ApproxEq<T>(dir.Length(), T(1), T(1e-4)));
            static const T COEF = Sqrt(3 / (4 * PI<T>));
            return COEF * dir.x;
        }
    };

    template<typename T> struct SHAux<T, 2, -2>
    {
        static T Eval(const Vec3<T> &dir)
        {
            AGZ_ASSERT(ApproxEq<T>(dir.Length(), T(1), T(1e-4)));
            static const T COEF = T(0.5) * Sqrt(15 / PI<T>);
            return COEF * dir.x * dir.y;
        }
    };

    template<typename T> struct SHAux<T, 2, -1>
    {
        static T Eval(const Vec3<T> &dir)
        {
            AGZ_ASSERT(ApproxEq<T>(dir.Length(), T(1), T(1e-4)));
            static const T COEF = T(0.5) * Sqrt(15 / PI<T>);
            return COEF * dir.y * dir.z;
        }
    };

    template<typename T> struct SHAux<T, 2, 0>
    {
        static T Eval(const Vec3<T> &dir)
        {
            AGZ_ASSERT(ApproxEq<T>(dir.Length(), T(1), T(1e-4)));
            static const T COEF = T(0.25) * Sqrt(5 / PI<T>);
            return COEF * (-dir.x * dir.x - dir.y * dir.y + 2 * dir.z * dir.z);
        }
    };

    template<typename T> struct SHAux<T, 2, 1>
    {
        static T Eval(const Vec3<T> &dir)
        {
            AGZ_ASSERT(ApproxEq<T>(dir.Length(), T(1), T(1e-4)));
            static const T COEF = T(0.5) * Sqrt(15 / PI<T>);
            return COEF * dir.z * dir.x;
        }
    };

    template<typename T> struct SHAux<T, 2, 2>
    {
        static T Eval(const Vec3<T> &dir)
        {
            AGZ_ASSERT(ApproxEq<T>(dir.Length(), T(1), T(1e-4)));
            static const T COEF = T(0.25) * Sqrt(15 / PI<T>);
            return COEF * (dir.x * dir.x - dir.y * dir.y);
        }
    };

    template<typename T, int L> struct PAux { };

    template<typename T> struct PAux<T, 1>
    {
        static Vec3<T> Eval(const Vec3<T> &dir)
        {
            static const T C = Sqrt(3 / (4 * PI<T>));
            auto r = dir.Length();
            return Vec3<T>(
                C * dir.y / r,
                C * dir.z / r,
                C * dir.x / r
            );
        }
    };
}

/**
 * @endcond
 */

/**
 * @brief 三维欧氏坐标系下的球谐函数，只定义到3阶
 * 
 * @param dir 单位方向向量
 */
template<typename T, int L, int M>
T SH(const Vec3<T> &dir)
{
    static_assert(M <= L);
    return SHImpl::SHAux<T, L, M>::Eval(dir);
}

/**
 * @brief 给定L和M，取得SH函数指针
 * 
 * 对超出实现范围的L和M值，返回零函数
 */
template<typename T>
auto GetSHByLM(int L, int M)
{
    using FuncPtr = T(*)(const Vec3<T>&);
    static FuncPtr FUNC_PTR[] =
    {
        &SH<T, 0, 0>,
        &SH<T, 1, -1>,
        &SH<T, 1, 0>,
        &SH<T, 1, 1>,
        &SH<T, 2, -2>,
        &SH<T, 2, -1>,
        &SH<T, 2, 0>,
        &SH<T, 2, 1>,
        &SH<T, 2, 2>,
    };
    int idx = L * (L + 1) + M;
    if(idx >= 9 || Abs(M) > L)
        return &SHImpl::Zero<T>;
    return FUNC_PTR[idx];
}

/**
 * @brief 旋转l=0对应的SH函数系数
 * 
 * @param M 旋转矩阵
 * @param SHCoef 待修改的系数
 */
template<typename T>
void RotateSH_L0([[maybe_unused]] const Mat3<T> &M, [[maybe_unused]] T *SHCoef)
{
    AGZ_ASSERT(SHCoef);
    // l=0对应的SH是常量函数，其系数不受旋转的影响
}

/**
 * @brief 旋转l=1对应的SH函数系数
 * 
 * 推导见 https://airguanz.github.io/2018/11/20/SH-PRT.html
 * 
 * @param M 旋转矩阵
 * @param SHCoef 待修改的系数，应包含3个元素
 */
template<typename T>
void RotateSH_L1(const Mat3<T> &M, T *SHCoef)
{
    static const T C     = Sqrt(3 / (4 * PI<T>));
    static const T INV_C = Sqrt(4 * PI<T> / 3);

    static const Mat3<T> INV_A(T(0),  T(0),  INV_C,
                               INV_C, T(0),  T(0),
                               T(0),  INV_C, T(0));
    
    // 由于N_i被选取为各轴上的方向向量，故M * Ni就是M的第i列

    auto PMN0 = SHImpl::PAux<T, 1>::Eval(M.GetCol(0));
    auto PMN1 = SHImpl::PAux<T, 1>::Eval(M.GetCol(1));
    auto PMN2 = SHImpl::PAux<T, 1>::Eval(M.GetCol(2));

    auto S = Mat3<T>::FromCols(PMN0, PMN1, PMN2);
    Vec3<T> x(SHCoef[0], SHCoef[1], SHCoef[2]);

    return S * (INV_A * x);
}

} // namespace AGZ::Math
