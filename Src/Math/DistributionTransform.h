#pragma once

/**
 * @file Math/DistributionTransform.h
 * @brief 常见分布转换，如从[0, 1]^2上的均匀分布转换为单位球面上的均匀分布等
 * 
 * - 将[0, 1]^2上的均匀分布转换到单位圆内的均匀分布
 * - 将[0, 1]^2上的均匀分布转换为半立体角上的均匀分布
 * - 将[0, 1]^2上的均匀分布转换为球面立体角上的均匀分布
 * - 将[0, 1]^2上的均匀分布转换为半立体角上的Z线性加权分布
 * - 将[0, 1]^2上的均匀分布转换为三角形上的均匀分布
 * - 将[0, 1]^2上的均匀分布转换为锥形立体角上的均匀分布
 * - 从[0, 1]上的均匀分布中额外抽出一个整型随机数
 */

#include <type_traits>

#include "Scalar.h"
#include "Vec2.h"
#include "Vec3.h"

namespace AGZ::Math::DistributionTransform {

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>> class UniformOnUnitDisk;
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>> class UniformOnUnitHemisphere;
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>> class UniformOnUnitSphere;
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>> class ZWeightedOnUnitHemisphere;
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>> class UniformOnTriangle;
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>> class UniformOnCone;
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>> class SampleExtractor;

/**
 * @brief 在单位圆内均匀采样
 */
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
class UniformOnUnitDisk
{
public:

    /** 采样结果 */
    struct Result
    {
        Vec2<T> sample; ///< 采样点
        T pdf;          ///< 采样点所对应的概率密度函数值
    };

    /** 把一对[0, 1]间的随机数转换为在单位圆内的均匀采样 */
    static Result Transform(const Vec2<T> &u) noexcept
    {
        T r = Sqrt(u.u);
        T theta = 2 * PI<T> * u.v;
        return { { r * Cos(theta), r*  Sin(theta) }, InvPI<T> };
    }

    /** 概率密度函数值 */
    static T PDF() noexcept
    {
        return InvPI<T>;
    }
};

/**
 * @brief 在单位半球面上均匀采样
 */
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
class UniformOnUnitHemisphere
{
public:

    /** 采样结果 */
    struct Result
    {
        Vec3<T> sample; ///< 采样点
        T pdf;          ///< 采样点对应的概率密度函数值
    };

    /** 把一对[0, 1]间的随机数转换为单位半球面上的均匀采样 */
    static Result Transform(const Vec2<T> &u) noexcept
    {
        T z = u.u;
        T phi = 2 * PI<T> * u.v;
        T r = Sqrt(Max(T(0), 1 - z * z));
        T x = r * Cos(phi);
        T y = r * Sin(phi);
        return { { x, y, z }, Inv2PI<T> };
    }

    /** 概率密度函数值 */
    static T PDF() noexcept
    {
        return Inv2PI<T>;
    }
};

/**
 * @brief 在单位球面上均匀采样
 */
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
class UniformOnUnitSphere
{
public:

    /** 采样结果 */
    struct Result
    {
        Vec3<T> sample; ///< 采样点
        T pdf;          ///< 采样点对应的概率密度函数值
    };

    /** 把一对[0, 1]间的随机数转换为单位球面上的均匀采样 */
    static Result Transform(const Vec2<T> &u) noexcept
    {
        T z = 1 - 2 * u.u;
        T phi = 2 * PI<T> * u.v;
        T r = Sqrt(Max(T(0), 1 - z * z));
        T x = r * Cos(phi);
        T y = r * Sin(phi);
        return { { x, y, z }, Inv4PI<T> };
    }

    /** 概率密度函数值 */
    static T PDF() noexcept
    {
        return Inv4PI<T>;
    }
};

/**
 * @brief 在单位半球面上进行Cos-weighed采样
 */
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
class ZWeightedOnUnitHemisphere
{
public:

    /** 采样结果 */
    struct Result
    {
        Vec3<T> sample; ///< 采样点
        T pdf;          ///< 采样点对应的概率密度函数值
    };

    /** 把一对[0, 1]间的随机数转换为单位半球面上的Cos-weighed采样 */
    static Result Transform(Vec2<T> u) noexcept
    {
        Vec2<T> sam;
        u = T(2) * u - Vec2<T>(1);
        if(u.x == 0 && u.y == 0)
            sam = Vec2<T>(T(0));
        else
        {
            T theta, r;
            if(Abs(u.x) > Abs(u.y)) {
                r = u.x;
                theta = T(0.25) * PI<T> * (u.y / u.x);
            }
            else {
                r = u.y;
                theta = T(0.5) * PI<T> - T(0.25) * PI<T> * (u.x / u.y);
            }
            sam = r * Vec2<T>(Cos(theta), Sin(theta));
        }

        T z = Sqrt(Max(T(0), 1 - sam.LengthSquare()));
        return { { sam.x, sam.y, z }, z * InvPI<T> };
    }

    /** 给定采样点，返回它在该采样策略中的概率密度函数值 */
    static T PDF(const Vec3<T> &sample) noexcept
    {
        return sample.z * InvPI<T>;
    }
};

/**
 * @brief 在三角形上均匀采样
 */
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
class UniformOnTriangle
{
public:

    /** 将一对[0, 1]间的随机数转换为三角坐标均匀采样 */
    static Vec2<T> Transform(const Vec2<T> &u) noexcept
    {
        T t = Sqrt(u.u);
        return { 1 - t, t * u.v };
    }
};

/**
 * @brief 在单位球面上具有一定顶角的范围内采样，且在立体角意义上是均匀的
 */
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
class UniformOnCone
{
public:

    /** 采样结果 */
    struct Result
    {
        Vec3<T> sample; ///< 采样点
        T pdf;            ///< 采样点对应的概率密度函数值
    };

    /**
     * 把一对[0, 1]间的随机数转换为以maxCosTheta为顶角的单位球面上的锥体内的方向采样，
     * 且在立体角意义上是均匀的
     */
    Result Transform(T maxCosTheta, const Vec2<T> &u) noexcept
    {
        T cosTheta = (1 - u.u) + u.u * maxCosTheta;
        T sinTheta = Sqrt(Max(T(0), T(1) - cosTheta * cosTheta));
        T phi = 2 * PI<T> * u.v;
        return {
            { Cos(phi) * sinTheta, Sin(phi) * sinTheta, cosTheta },
            1 / (2 * PI<T> * (1 - maxCosTheta))
        };
    }

    /** 概率密度函数值 */
    static T PDF(T maxCosTheta) noexcept
    {
        return 1 / (2 * PI<T> * (1 - maxCosTheta));
    }
};

/**
 * @brief 随机数萃取器，将一个[0, 1]间的均匀随机数转换为一个整数和一个随机数
 */
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
class SampleExtractor
{
public:

    template<typename I, std::enable_if_t<std::is_integral_v<I>, int> = 0>
    static std::tuple<I, T> ExtractInteger(T u, I begin, I end) noexcept
    {
        AGZ_ASSERT(begin < end);
        I delta = end - begin;
        I integer = begin + (std::min<I>)(I(u * delta), delta - 1);
        T real = (std::min<T>)(begin + u * delta - integer, 1);
        return { integer, real };
    }
};

} // namespace AGZ::Math::DistributionTransform
