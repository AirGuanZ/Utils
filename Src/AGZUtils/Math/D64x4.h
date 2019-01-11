#pragma once

/**
 * @file Math/D64x4.h
 * @brief 定义了使用AVX加速的四维double向量，仅在定义了AGZ_USE_AUX宏时有效
 */

#include "../Misc/Common.h"

#if defined(AGZ_USE_AVX)

#include <immintrin.h>

#include "../Math/Vec4.h"

namespace AGZ::Math {

/**
 * @brief 使用AVX加速的四double向量
 */
class alignas(32) D64x4
{
public:

    union
    {
        __m256d d;
        double data[4];
        struct { double x, y, z, w; };
        struct { double r, g, b, a; };
        struct { double u, v, m, n; };
    };

public:

    using Component = double;
    using Self = D64x4;

    /** 所有元素默认初始化为0 */
    D64x4()
        : d(_mm256_setzero_pd())
    {

    }

    /** 不对元素进行初始化 */
    explicit D64x4(Uninitialized_t)
    {

    }

    /** 分别初始化四个元素 */
    D64x4(double x, double y, double z, double w)
        : d(_mm256_set_pd(w, z, y, x))
    {

    }

    /** 将所有元素初始化为同一个值 */
    explicit D64x4(double v)
        : d(_mm256_set1_pd(v))
    {

    }

    /** 提供指向初始化数据的指针 */
    explicit D64x4(double *data)
        : d(_mm256_load_pd(data))
    {

    }

    /** 从__m256d初始化 */
    explicit D64x4(const __m256d other)
        : d(other)
    {

    }

    /** 从Vec4d初始化 */
    explicit D64x4(const Vec4d &v)
        : d(_mm256_load_pd(&v.x))
    {

    }

    /** 值复制 */
    D64x4(const Self &other)
        : d(other.d)
    {

    }

    /** 转换为Vec4d */
    Vec4d AsVec() const
    {
        return Vec4d(data);
    }

    /** 转换为Vec4d */
    explicit operator Vec4d() const
    {
        return AsVec();
    }

    /** 值复制 */
    Self &operator=(const Self &other)
    {
        d = other.d;
        return *this;
    }

    /** 按下标取元素，下标范围[0, 4) */
    double &operator[](size_t idx) { AGZ_ASSERT(idx < 4); return data[idx]; }
    /** 按下标取元素，下标范围[0, 4) */
    double operator[](size_t idx) const { AGZ_ASSERT(idx < 4); return data[idx]; }

    /** 每个元素分别相加 */
    Self operator+(const Self &rhs) const { return Self(_mm256_add_pd(d, rhs.d)); }
    /** 每个元素分别相减 */
    Self operator-(const Self &rhs) const { return Self(_mm256_sub_pd(d, rhs.d)); }
    /** 每个元素分别相乘 */
    Self operator*(const Self &rhs) const { return Self(_mm256_mul_pd(d, rhs.d)); }
    /** 每个元素分别相除 */
    Self operator/(const Self &rhs) const { return Self(_mm256_div_pd(d, rhs.d)); }

    /** 是否每个元素都比rhs的对应元素小 */
    bool ElemwiseLessThan(const Self &rhs) const { return _mm256_movemask_pd(_mm256_cmp_pd(d, rhs.d, 1)) == 0x0f; }
};

static_assert(sizeof(D64x4) == 32 && alignof(D64x4) == 32);

/** 每个元素分别取绝对值 */
inline D64x4 Abs(D64x4 v)
{
    return D64x4(_mm256_and_pd(v.d, _mm256_castsi256_pd(_mm256_set1_epi64x(0x7fffffffffffffff))));
}

/** 每个元素分别取更小的那个 */
inline D64x4 Min(D64x4 lhs, D64x4 rhs)
{
    return D64x4(_mm256_min_pd(lhs.d, rhs.d));
}

/** 每个元素分别取更大的那个 */
inline D64x4 Max(D64x4 lhs, D64x4 rhs)
{
    return D64x4(_mm256_max_pd(lhs.d, rhs.d));
}

/** 每个元素分别开平方 */
inline D64x4 Sqrt(D64x4 v)
{
    return D64x4(_mm256_sqrt_pd(v.d));
}

/** 每个元素都在epsilon容忍度下近似相等 */
inline bool ApproxEq(D64x4 lhs, D64x4 rhs, double epsilon)
{
    return Abs(lhs - rhs).ElemwiseLessThan(D64x4(epsilon));
}

/** 取前三个元素的点乘 */
inline double Dot3(D64x4 lhs, D64x4 rhs)
{
    D64x4 r = lhs * rhs;
    return r.x + r.y + r.z;
}

/** 点乘 */
inline double Dot(D64x4 lhs, D64x4 rhs)
{
    D64x4 r = lhs * rhs;
    return r.x + r.y + r.z + r.w;
}

} // namespace AGZ::Math

#endif
