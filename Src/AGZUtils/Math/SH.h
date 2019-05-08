#pragma once

#include "../Misc/Common.h"
#include "RM_Mat3.h"
#include "Scalar.h"
#include "Vec.h"
#include "Vec3.h"

namespace AGZ::Math::SH {

/**
 * @cond
 */

namespace SHImpl
{
    template<typename T>
    using SH_FUNC_PTR = T(*)(const Vec3<T>&) noexcept;

    // 见 https://en.wikipedia.org/wiki/Table_of_spherical_harmonics
    template<typename T, int L, int M> struct SHAux { };

    template<typename T>
    T Zero([[maybe_unused]] const Vec3<T>&) noexcept
    {
        return T(0);
    }

#define DEF_SH(L, M, COEF, RET) \
    template<typename T> struct SHAux<T, L, M> \
    { \
        static T Eval([[maybe_unused]] const Vec3<T> &dir) noexcept \
        { \
            AGZ_ASSERT(ApproxEq<T>(dir.Length(), T(1), T(1e-4))); \
            [[maybe_unused]] constexpr T pi = ::AGZ::Math::PI<T>; \
            [[maybe_unused]] T x = dir.x; \
            [[maybe_unused]] T y = dir.y; \
            [[maybe_unused]] T z = dir.z; \
            static const T C = (COEF); \
            return C * (RET); \
        } \
    }

    DEF_SH(0, 0, T(0.5) * Sqrt(1 / pi), 1);
    
    DEF_SH(1, -1, Sqrt(3 / (4 * pi)), y);
    DEF_SH(1, 0,  Sqrt(3 / (4 * pi)), z);
    DEF_SH(1, 1,  Sqrt(3 / (4 * pi)), x);

    DEF_SH(2, -2, T(0.5)  * Sqrt(15 / pi), x * y);
    DEF_SH(2, -1, T(0.5)  * Sqrt(15 / pi), y * z);
    DEF_SH(2, 0,  T(0.25) * Sqrt(5 / pi),  - x * x - y * y + 2 * z * z);
    DEF_SH(2, 1,  T(0.5)  * Sqrt(15 / pi), z * x);
    DEF_SH(2, 2,  T(0.25) * Sqrt(15 / pi), x * x - y * y);

    DEF_SH(3, -3, T(0.25) * Sqrt(35 / (2 * pi)), (3 * x * x - y * y) * y);
    DEF_SH(3, -2, T(0.5)  * Sqrt(105 / pi),      x * y * z);
    DEF_SH(3, -1, T(0.25) * Sqrt(21 / (2 * pi)), y * (4 * z * z - x * x - y * y));
    DEF_SH(3, 0,  T(0.25) * Sqrt(7 / pi),        z * (2 * z * z - 3 * x * x - 3 * y * y));
    DEF_SH(3, 1,  T(0.25) * Sqrt(21 / (2 * pi)), x * (4 * z * z - x * x - y * y));
    DEF_SH(3, 2,  T(0.25) * Sqrt(105 / pi),      (x * x - y * y) * z);
    DEF_SH(3, 3,  T(0.25) * Sqrt(35 / (2 * pi)), (x * x - 3 * y * y) * x);

    DEF_SH(4, -4, T(0.75)   * Sqrt(35 / pi),       x * y * (x * x - y * y));
    DEF_SH(4, -3, T(0.75)   * Sqrt(35 / (2 * pi)), (3 * x * x - y * y) * y * z);
    DEF_SH(4, -2, T(0.75)   * Sqrt(5 / pi),        x * y * (7 * z * z - 1));
    DEF_SH(4, -1, T(0.75)   * Sqrt(5 / (2 * pi)),  y * z * (7 * z * z - 3));
    DEF_SH(4, 0,  T(3) / 16 * Sqrt(1 / pi),        (35 * (z * z) * (z * z) - 30 * z * z + 3));
    DEF_SH(4, 1,  T(0.75)   * Sqrt(5 / (2 * pi)),  x * z * (7 * z * z - 3));
    DEF_SH(4, 2,  T(3) / 8  * Sqrt(5 / pi),        (x * x - y * y) * (7 * z * z - 1));
    DEF_SH(4, 3,  T(0.75)   * Sqrt(35 / (2 * pi)), (x * x - 3 * y * y) * x * z);
    DEF_SH(4, 4,  T(3) / 16 * Sqrt(35 / pi),       x * x * (x * x - 3 * y * y) - y * y * (3 * x * x - y * y));

#undef DEF_SH

    // 投影函数P，见 https://airguanz.github.io/2018/11/20/SH-PRT.html
    template<typename T, int L> struct PAux { };

    template<typename T> struct PAux<T, 1>
    {
        static Vec3<T> Eval(const Vec3<T> &dir) noexcept
        {
            static const T C = Sqrt(3 / (4 * PI<T>));
            auto r = dir.Length();
            return Vec3<T>(C * dir.y / r, C * dir.z / r, C * dir.x / r
            );
        }
    };

    template<typename T> struct PAux<T, 2>
    {
        static Vec<5, T> Eval(const Vec3<T> &dir) noexcept
        {
            auto ndir = dir.Normalize();
            return Vec<5, T>(
                SHAux<T, 2, -2>::Eval(ndir),
                SHAux<T, 2, -1>::Eval(ndir),
                SHAux<T, 2, 0> ::Eval(ndir),
                SHAux<T, 2, 1> ::Eval(ndir),
                SHAux<T, 2, 2> ::Eval(ndir)
            );
        }
    };

    template<typename T> struct PAux<T, 3>
    {
        static Vec<7, T> Eval(const Vec3<T> &dir) noexcept
        {
            auto ndir = dir.Normalize();
            return Vec<7, T>(
                SHAux<T, 3, -3>::Eval(ndir),
                SHAux<T, 3, -2>::Eval(ndir),
                SHAux<T, 3, -1>::Eval(ndir),
                SHAux<T, 3, 0> ::Eval(ndir),
                SHAux<T, 3, 1> ::Eval(ndir),
                SHAux<T, 3, 2> ::Eval(ndir),
                SHAux<T, 3, 3> ::Eval(ndir)
            );
        }
    };

    template<typename T> struct PAux<T, 4>
    {
        static Vec<9, T> Eval(const Vec3<T> &dir) noexcept
        {
            auto ndir = dir.Normalize();
            return Vec<9, T>(
                SHAux<T, 4, -4>::Eval(ndir),
                SHAux<T, 4, -3>::Eval(ndir),
                SHAux<T, 4, -2>::Eval(ndir),
                SHAux<T, 4, -1>::Eval(ndir),
                SHAux<T, 4, 0> ::Eval(ndir),
                SHAux<T, 4, 1> ::Eval(ndir),
                SHAux<T, 4, 2> ::Eval(ndir),
                SHAux<T, 4, 3> ::Eval(ndir),
                SHAux<T, 4, 4> ::Eval(ndir)
            );
        }
    };
}

/**
 * @endcond
 */

/**
 * @brief 三维欧氏坐标系下的球谐函数，只定义到5阶
 * 
 * @param dir 单位方向向量
 */
template<typename T, int L, int M>
T SH(const Vec3<T> &dir) noexcept
{
    static_assert(M <= L && L <= 4);
    return SHImpl::SHAux<T, L, M>::Eval(dir);
}

/**
 * @brief 取得用L * (L + 1) + M作为索引的SH函数指针表
 * 
 * 仅支持实现范围内的L和M值，越界 == UB
 */
template<typename T>
SHImpl::SH_FUNC_PTR<T> *GetSHTable() noexcept
{
    // SH bands tower!
    static SHImpl::SH_FUNC_PTR<T> FUNC_PTR[] =
    {
                                                                    &SH<T, 0, 0>,
                                                     &SH<T, 1, -1>, &SH<T, 1, 0>, &SH<T, 1, 1>,
                                      &SH<T, 2, -2>, &SH<T, 2, -1>, &SH<T, 2, 0>, &SH<T, 2, 1>, &SH<T, 2, 2>,
                       &SH<T, 3, -3>, &SH<T, 3, -2>, &SH<T, 3, -1>, &SH<T, 3, 0>, &SH<T, 3, 1>, &SH<T, 3, 2>, &SH<T, 3, 3>,
        &SH<T, 4, -4>, &SH<T, 4, -3>, &SH<T, 4, -2>, &SH<T, 4, -1>, &SH<T, 4, 0>, &SH<T, 4, 1>, &SH<T, 4, 2>, &SH<T, 4, 3>, &SH<T, 4, 4>,
    };
    return FUNC_PTR;
}

/**
 * @brief 给定L和M，取得SH函数指针
 * 
 * 对超出实现范围的L和M值，返回零函数
 */
template<typename T>
auto GetSHByLM(int L, int M) noexcept
{
    if(L > 4 || Abs(M) > L)
        return &SHImpl::Zero<T>;
    int idx = L * (L + 1) + M;
    return GetSHTable<T>()[idx];
}

/**
 * @brief 旋转l=0对应的SH函数系数
 * 
 * @param M 旋转矩阵
 * @param SHCoef 待修改的系数
 */
template<typename T>
void RotateSH_L0([[maybe_unused]] const RM_Mat3<T> &M, [[maybe_unused]] T *SHCoef) noexcept
{
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
void RotateSH_L1(const RM_Mat3<T> &M, T *SHCoef) noexcept
{
    static const T INV_C = Sqrt(4 * PI<T> / 3);

    static const RM_Mat3<T> INV_A(T(0),  T(0),  INV_C,
                                  INV_C, T(0),  T(0),
                                  T(0),  INV_C, T(0));
    
    // 由于N_i被选取为各轴上的方向向量，故M * Ni就是M的第i列

    auto PMN0 = SHImpl::PAux<T, 1>::Eval(M.GetCol(0));
    auto PMN1 = SHImpl::PAux<T, 1>::Eval(M.GetCol(1));
    auto PMN2 = SHImpl::PAux<T, 1>::Eval(M.GetCol(2));
    auto S = RM_Mat3<T>::FromCols(PMN0, PMN1, PMN2);

    Vec3<T> x(SHCoef[0], SHCoef[1], SHCoef[2]);
    x = S * (INV_A * x);

    SHCoef[0] = x[0];
    SHCoef[1] = x[1];
    SHCoef[2] = x[2];
}

/**
 * @brief 旋转l=2对应的SH函数系数
 * 
 * 推导见 https://airguanz.github.io/2018/11/20/SH-PRT.html
 * 
 * @param M 旋转矩阵
 * @param SHCoef 待修改的系数，应包含5个元素
 */
template<typename T>
void RotateSH_L2(const RM_Mat3<T> &M, T *SHCoef) noexcept
{
    constexpr T K = T(0.7071067811865475);
    static const Vec3<T> N2(K, K, T(0));
    static const Vec3<T> N3(K, T(0), K);
    static const Vec3<T> N4(T(0), K, K);

    constexpr T K0 = T(0.91529123286551084);
    constexpr T K1 = T(1.83058246573102168);
    constexpr T K2 = T(1.5853309190550713);

    // 计算A^{-1}x

    Vec<5, T> invAx(K0 * (SHCoef[1] - SHCoef[3]) + K1 * SHCoef[4],
                    K0 * (SHCoef[0] - SHCoef[3] + SHCoef[4]) + K2 * SHCoef[2],
                    K1 * SHCoef[0],
                    K1 * SHCoef[3],
                    K1 * SHCoef[1]);

    // 构造S
    // IMPROVE：N是确定的，故P(M * N)可以优化

    auto PMN0 = SHImpl::PAux<T, 2>::Eval(M.GetCol(0));
    auto PMN1 = SHImpl::PAux<T, 2>::Eval(M.GetCol(2));
    auto PMN2 = SHImpl::PAux<T, 2>::Eval(M * N2);
    auto PMN3 = SHImpl::PAux<T, 2>::Eval(M * N3);
    auto PMN4 = SHImpl::PAux<T, 2>::Eval(M * N4);

    // 计算S(A^{-1}x)

    SHCoef[0] = PMN0[0] * invAx[0] + PMN1[0] * invAx[1] + PMN2[0] * invAx[2] + PMN3[0] * invAx[3] + PMN4[0] * invAx[4];
    SHCoef[1] = PMN0[1] * invAx[0] + PMN1[1] * invAx[1] + PMN2[1] * invAx[2] + PMN3[1] * invAx[3] + PMN4[1] * invAx[4];
    SHCoef[2] = PMN0[2] * invAx[0] + PMN1[2] * invAx[1] + PMN2[2] * invAx[2] + PMN3[2] * invAx[3] + PMN4[2] * invAx[4];
    SHCoef[3] = PMN0[3] * invAx[0] + PMN1[3] * invAx[1] + PMN2[3] * invAx[2] + PMN3[3] * invAx[3] + PMN4[3] * invAx[4];
    SHCoef[4] = PMN0[4] * invAx[0] + PMN1[4] * invAx[1] + PMN2[4] * invAx[2] + PMN3[4] * invAx[3] + PMN4[4] * invAx[4];
}

/**
 * @brief 旋转l=3对应的SH函数系数
 *
 * 原理和RotateSH_L2一样
 *
 * @param M 旋转矩阵
 * @param SHCoef 待修改的系数，应包含7个元素
 */
template<typename T>
void RotateSH_L3(const RM_Mat3<T> &M, T *SHCoef) noexcept
{
    static const Vec3<T> N[7] = {
           Vec3<T>(T(1),   T(0), T(0))   .Normalize(),
           Vec3<T>(T(0),   T(1), T(0))   .Normalize(),
           Vec3<T>(T(0.3), T(0), T(1))   .Normalize(),
           Vec3<T>(T(0),   T(1), T(1))   .Normalize(),
           Vec3<T>(T(1),   T(0), T(1))   .Normalize(),
           Vec3<T>(T(1),   T(1), T(0.78)).Normalize(),
           Vec3<T>(T(1),   T(1), T(1))   .Normalize()
    };

    static const T invA[7][7] = {
        { T(  0.707711955885399), T( 0.643852929494021), T( -0.913652206352009), T(-0.093033334712756), T(  0.328680372803511), T( -1.131667680791894), T(  1.949384763080401) },
        { T( -1.114187338255984), T( 0.643852929494021), T( -0.749554866243252), T(-0.093033334712757), T(  0.164583032694754), T( -0.232204002745663), T(  0.127485468939019) },
        { T(  2.296023687102124), T(                 0), T( -2.964153834214758), T(                 0), T(  2.964153834214758), T( -3.749390980495911), T(  2.296023687102124) },
        { T(  2.392306681179504), T(-1.099424142052695), T( -3.088454645076318), T(-2.129025696294232), T(  3.766408103751610), T( -5.313883353254694), T(  2.917447172170129) },
        { T(  1.878707739441422), T(-1.099424142052695), T( -2.425401262415870), T(-2.129025696294233), T(  3.103354721091161), T( -2.518204820606409), T(  2.403848230432046) },
        { T( 13.656934981397061), T(-4.181565269348606), T(-17.631027247729438), T(-8.097566324633245), T( 14.325209638780166), T(-20.210898801851609), T( 11.096259672385109) },
        { T(-13.139185354460187), T( 5.820633765367933), T( 16.962615353518899), T( 7.790578559853934), T(-13.782124974734103), T( 19.444681101542464), T(-10.675588100498899) },
    };

    Vec<7, T> invAx(T(0));
    for(int i = 0; i != 7; ++i)
    {
        for(int j = 0; j != 7; ++j)
            invAx[i] += invA[i][j] * SHCoef[j];
    }

    Vec<7, T> PMN[7];
    for(int i = 0; i < 7; ++i)
        PMN[i] = SHImpl::PAux<T, 3>::Eval(M * N[i]);

    for(int i = 0; i < 7; ++i)
    {
        SHCoef[i] = T(0);
        for(int j = 0; j < 7; ++j)
            SHCoef[i] += PMN[j][i] * invAx[j];
    }
}

/**
* @brief 旋转l=4对应的SH函数系数
*
* 原理和RotateSH_L2一样
*
* @param M 旋转矩阵
* @param SHCoef 待修改的系数，应包含9个元素
*/
template<typename T>
void RotateSH_L4(const RM_Mat3<T> &M, T *SHCoef) noexcept
{
    static const Vec3<T> N[9] = {
           Vec3<T>(T(1),    T(0),    T(0))   .Normalize(),
           Vec3<T>(T(0),    T(1),    T(0))   .Normalize(),
           Vec3<T>(T(0.3),  T(0),    T(1))   .Normalize(),
           Vec3<T>(T(0),    T(1),    T(1))   .Normalize(),
           Vec3<T>(T(1),    T(0),    T(1))   .Normalize(),
           Vec3<T>(T(1),    T(0.54), T(0.78)).Normalize(),
           Vec3<T>(T(1),    T(1),    T(0.78)).Normalize(),
           Vec3<T>(T(0.31), T(1),    T(0.78)).Normalize(),
           Vec3<T>(T(1),    T(1),    T(1))   .Normalize()
    };

    static const T invA[9][9] = {
        { T(-1.948206991589258), T(1.912687049138671),  T(-0.763091021186035), T(-0.286837642392582), T(-0.341264679278342), T(0.594477634079894),  T(-1.056887279361603), T(0.578857155270682),  T(0.971984464556520)  },
        { T(2.171192074917378),  T(-0.142084581369102), T(-1.577618721617938), T(0.828536347413562),  T(-0.705532540822805), T(0.382031320127708),  T(1.056887279361603),  T(-2.513802449733083), T(1.156701984383617)  },
        { T(2.053952330860290),  T(-0.094158653118148), T(-0.750956907863241), T(-1.098731135021785), T(-0.335838138831051), T(1.931188736063331),  T(0),                  T(-1.051043414216722), T(0.170301019159901)  },
        { T(3.993132334888566),  T(1.179414191911931),  T(-4.808985771815311), T(1.266884703225481),  T(-3.095952538204609), T(2.811562290853012),  T(0),                  T(-4.022967497037739), T(1.569934476060706)  },
        { T(-1.543780567538975), T(1.894449743774703),  T(-2.499709102566265), T(-0.207318037527907), T(-2.063212615945576), T(1.725864595116423),  T(0),                  T(-0.365404044003703), T(1.046239752465574)  },
        { T(3.435134010827782),  T(-2.932684025967419), T(4.231264528651311),  T(-2.972023260715974), T(1.892279023369589),  T(-1.718456688280952), T(0),                  T(2.458880397035034),  T(-0.959560600640598) },
        { T(3.689266412234284),  T(1.985158283498190),  T(-7.403078714786565), T(-3.123392326177335), T(-3.310757449808909), T(3.006635497533013),  T(0),                  T(-4.302091019418769), T(1.678860447048080)  },
        { T(-0.367659806642012), T(-3.222124483746851), T(4.648868038376401),  T(-3.265346293642776), T(2.079036990447149),  T(-1.888059306949047), T(0),                  T(2.701558933638689),  T(-1.054264174928627) },
        { T(-4.515212732000947), T(3.220651333447782),  T(0.208527587656698),  T(6.066568738154828),  T(-0.970215938306426), T(0.881093140952614),  T(0),                  T(-1.260725782049042), T(0.491989276959057)  },
    };

    Vec<9, T> invAx(T(0));
    for(int i = 0; i != 9; ++i)
    {
        for(int j = 0; j != 9; ++j)
            invAx[i] += invA[i][j] * SHCoef[j];
    }

    Vec<9, T> PMN[9];
    for(int i = 0; i < 9; ++i)
        PMN[i] = SHImpl::PAux<T, 4>::Eval(M * N[i]);

    for(int i = 0; i < 9; ++i)
    {
        SHCoef[i] = T(0);
        for(int j = 0; j < 9; ++j)
            SHCoef[i] += PMN[j][i] * invAx[j];
    }
}

} // namespace AGZ::Math::SH
