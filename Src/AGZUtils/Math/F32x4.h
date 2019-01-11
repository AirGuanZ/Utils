#pragma once

/**
 * @file Math/F32x4.h
 * @brief 定义了使用SSE加速的四维float向量，仅在定义了AGZ_USE_SSE2宏时有效
 */

#include "../Misc/Common.h"

#if defined(AGZ_USE_SSE2)

#include <emmintrin.h>

#include "../Math/Vec4.h"

namespace AGZ::Math {

/**
 * @brief 使用SSE加速的四float向量
 */
class alignas(16) F32x4
{
public:

    union
    {
        __m128 d;
        float data[4];
        struct { float x, y, z, w; };
        struct { float r, g, b, a; };
        struct { float u, v, m, n; };
    };

public:

    using Component = float;
    using Self = F32x4;

    /** 所有元素默认初始化为0 */
    F32x4()
        : d(_mm_setzero_ps())
    {
        
    }

    /** 不对元素进行初始化 */
    explicit F32x4(Uninitialized_t)
    {
        
    }

    /** 分别初始化四个元素 */
    F32x4(float x, float y, float z, float w)
        : d(_mm_set_ps(w, z, y, x))
    {
        
    }

    /** 将所有元素初始化为同一个值 */
    explicit F32x4(float v)
        : d(_mm_set_ps1(v))
    {
        
    }

    /** 提供指向初始化数据的指针 */
    explicit F32x4(const float *data)
        : d(_mm_load_ps(data))
    {
        
    }

    /** 从__m128初始化 */
    explicit F32x4(const __m128 other)
        : d(other)
    {
        
    }

    /** 从Vec4f初始化 */
    explicit F32x4(const Vec4f &v)
        : d(_mm_load_ps(&v.x))
    {

    }

    /** 值复制 */
    F32x4(const Self &other)
        : d(other.d)
    {

    }

    /** 转换为Vec4f */
    explicit operator Vec4f() const
    {
        return AsVec();
    }

    /** 转换为Vec4f */
    Vec4f AsVec() const
    {
        return Vec4f(data);
    }

    /** 值复制 */
    Self &operator=(const Self &other)
    {
        d = other.d;
        return *this;
    }

    /** 按下标取元素，下标范围[0, 4) */
    float &operator[](size_t idx) { AGZ_ASSERT(idx < 4); return data[idx]; }
    /** 按下标取元素，下标范围[0, 4) */
    float operator[](size_t idx) const { AGZ_ASSERT(idx < 4); return data[idx]; }

    /** 每个元素分别相加 */
    Self operator+(const Self &rhs) const { return Self(_mm_add_ps(d, rhs.d)); }
    /** 每个元素分别相减 */
    Self operator-(const Self &rhs) const { return Self(_mm_sub_ps(d, rhs.d)); }
    /** 每个元素分别相乘 */
    Self operator*(const Self &rhs) const { return Self(_mm_mul_ps(d, rhs.d)); }
    /** 每个元素分别相除 */
    Self operator/(const Self &rhs) const { return Self(_mm_div_ps(d, rhs.d)); }

    /** 是否每个元素严格相等 */
    bool operator==(const Self &rhs) const { return _mm_movemask_ps(_mm_cmpeq_ps(d, rhs.d)) == 0x0f; }
    /** 是否存在不相等的对应元素 */
    bool operator!=(const Self &rhs) const { return !(*this == rhs); }

    /** 是否每个元素都比rhs的对应元素小 */
    bool ElemwiseLessThan(const Self &rhs) const { return _mm_movemask_ps(_mm_cmplt_ps(d, rhs.d)) == 0x0f; }
};

static_assert(sizeof(F32x4) == 16 && alignof(F32x4) == 16);

/** 每个元素分别取绝对值 */
inline F32x4 Abs(const F32x4 &v) { return F32x4(_mm_and_ps(v.d, _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff)))); }

/** 每个元素分别开平方 */
inline F32x4 Sqrt(const F32x4 &v) { return F32x4(_mm_sqrt_ps(v.d)); }

/** 每个元素都在epsilon容忍度下近似相等 */
inline bool ApproxEq(const F32x4 &lhs, const F32x4 &rhs, float epsilon)
{
    return Abs(lhs - rhs).ElemwiseLessThan(F32x4(epsilon));
}

} // namespace AGZ::Math

#endif
