#pragma once

/**
 * @file Math/DistributionTransform.h
 * @brief 常见分布转换
 * 
 * - 将[0, 1]^2上的均匀分布转换到单位圆内的均匀分布
 * - 将[0, 1]^2上的均匀分布转换为半立体角上的均匀分布
 * - 将[0, 1]^2上的均匀分布转换为球面立体角上的均匀分布
 * - 将[0, 1]^2上的均匀分布转换为半立体角上的Z线性加权分布
 * - 将[0, 1]^2上的均匀分布转换为三角形上的均匀分布
 * - 将[0, 1]^2上的均匀分布转换为锥形立体角上的均匀分布
 * - 从[0, 1]上的均匀分布中额外抽出一个整型随机数
 * - 将[0, 1]上的均匀分布转换为给定范围内的整数上的均匀分布
 * - 将[0, 1]间的均匀浮点数转换为服从给定了inverse CDF表格分布
 * - 将[0, 1]间的均匀浮点数转换为服从给定的一维分段线性常值cdf的分布
 * - 将[0, 1]^2上的均匀浮点数转换为服从给定的二维分段线性常值cdf的分布
 */

#include <type_traits>

#include "Scalar.h"
#include "Vec2.h"
#include "Vec3.h"

namespace AGZ::Math::DistributionTransform {

template<typename T> class UniformOnUnitDisk;
template<typename T> class UniformOnUnitHemisphere;
template<typename T> class UniformOnUnitSphere;
template<typename T> class ZWeightedOnUnitHemisphere;
template<typename T> class UniformOnTriangle;
template<typename T> class UniformOnCone;
template<typename T> class SampleExtractor;
template<typename T> class UniformInteger;
template<typename T> class TableSampler;
template<typename T> class PiecewiseConstantDistribution1D;
template<typename T> class PiecewiseConstantDistribution2D;

/**
 * @brief 在单位圆内均匀采样
 */
template<typename T>
class UniformOnUnitDisk
{
    static_assert(std::is_floating_point_v<T>);

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
template<typename T>
class UniformOnUnitHemisphere
{
    static_assert(std::is_floating_point_v<T>);

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
template<typename T>
class UniformOnUnitSphere
{
    static_assert(std::is_floating_point_v<T>);

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
template<typename T>
class ZWeightedOnUnitHemisphere
{
    static_assert(std::is_floating_point_v<T>);

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
template<typename T>
class UniformOnTriangle
{
    static_assert(std::is_floating_point_v<T>);

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
template<typename T>
class UniformOnCone
{
    static_assert(std::is_floating_point_v<T>);

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
    static Result Transform(T maxCosTheta, const Vec2<T> &u) noexcept
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
template<typename T>
class SampleExtractor
{
    static_assert(std::is_floating_point_v<T>);

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

/**
 * @brief 将[0, 1]间的浮点随机数转换为指定范围内的均匀整数
 */
template<typename T>
class UniformInteger
{
    static_assert(std::is_floating_point_v<T>);

public:

    template<typename I, std::enable_if_t<std::is_integral_v<I>, int> = 0>
    static I Transform(T u, I begin, I end) noexcept
    {
        return (std::min)(end - 1, begin + I(u * (end - begin)));
    }
};

/**
 * @brief 给定一张inverse CDF表格，将[0, 1]间的均匀浮点数转换为服从该CDF的浮点数
 * 
 * 记inverse CDF表格为A，大小为N，则A[0]对应CDF^{-1}(0)，A[N-1]对应CDF^{-1}(1)，中间的值用最近的表项作线性插值得到
 */
template<typename T>
class TableSampler
{
    static_assert(std::is_floating_point_v<T>);

public:

    static T Sample(T u, const T *invCDF, size_t tabSize) noexcept
    {
        AGZ_ASSERT(tabSize >= 2);
        T global = u * (tabSize - 1);
        size_t low = Min<size_t>(static_cast<size_t>(global), tabSize - 2);
        T local = global - low;
        return invCDF[low] * (1 - local) + invCDF[low + 1] * local;
    }
};

/**
 * @brief 将[0, 1]上的均匀浮点数转换为服从给定的等间距分段常值cdf的分布
 * 
 * 设一条线段被等间距地分为N段，每段有一个长度，记作w[0], w[1], ..., w[N-1]
 * 令tab[i] = w[0] + ... + n[i]，则tab[i]记录了第i段及其左边所有段的权值之和，tab[N-1]记录了全部权值之和
 * 现在在线段上以均匀分布随机选择一点，返回该点落在哪一段上，落在该段上的分布律值，以及具体落在该段上的哪个位置
 */
template<typename T>
class PiecewiseConstantDistribution1D
{
    static_assert(std::is_floating_point_v<T>);

public:

    struct Result
    {
        size_t value;
        T pdf;
        T insidePosition; // 采样点在该段内处于什么位置，取值范围[0, 1)
    };

    static Result Transform(T u, const T *tab, size_t tabSize) noexcept
    {
        AGZ_ASSERT(tab >= 1);

        u *= tab[tabSize - 1];

        auto lowIt = std::lower_bound(tab, tab + tabSize, u);
        size_t value = Min<size_t>(lowIt - tab, tabSize - 1);

        T pieceLength = value > 0 ? (tab[value] - tab[value-1]) : tab[value];
        T uOffset     = value > 0 ? (u          - tab[value-1]) : u;

        T pdf = pieceLength / tab[tabSize - 1];
        T insidePos = uOffset / pieceLength;

        return { value, pdf, insidePos };
    }

    static T PDF(size_t value, const T *tab, size_t tabSize) noexcept
    {
        AGZ_ASSERT(tabSize >= 1 && value < tabSize);
        T ret = value > 0 ? (tab[value] - tab[value-1]) : tab[value];
        return ret /= tab[tabSize - 1];
    }
};

template<typename T>
class PiecewiseConstantDistribution2D
{
    static_assert(std::is_floating_point_v<T>);

    // TODO
};

} // namespace AGZ::Math::DistributionTransform
