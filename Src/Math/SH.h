#pragma once

#include "../Misc/Common.h"
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

} // namespace AGZ::Math
