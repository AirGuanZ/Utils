#pragma once

#include "../Common.h"

#ifdef AGZ_USE_SSE

#include <emmintrin.h>

#include "../Math/Vec4.h"

AGZ_NS_BEG(AGZ::Math)

class alignas(16) f32x4
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
    using Self = f32x4;
    
    AGZ_FORCE_INLINE f32x4()
        : d(_mm_setzero_ps())
    {
        
    }

    explicit AGZ_FORCE_INLINE f32x4(Uninitialized_t)
    {
        
    }

    AGZ_FORCE_INLINE f32x4(float x, float y, float z, float w)
        : d(_mm_set_ps(w, z, y, x))
    {
        
    }

    explicit AGZ_FORCE_INLINE f32x4(float v)
        : d(_mm_set_ps1(v))
    {
        
    }

    explicit AGZ_FORCE_INLINE f32x4(const float *data)
        : d(_mm_load_ps(data))
    {
        
    }

    explicit AGZ_FORCE_INLINE f32x4(const __m128 other)
        : d(other)
    {
        
    }

    explicit AGZ_FORCE_INLINE f32x4(const Vec4f &v)
        : d(_mm_load_ps(&v.x))
    {

    }

    AGZ_FORCE_INLINE f32x4(const Self &other)
        : d(other.d)
    {

    }

    AGZ_FORCE_INLINE Vec4f AsVec() const
    {
        return Vec4f(data);
    }

    AGZ_FORCE_INLINE float &operator[](size_t idx) { AGZ_ASSERT(idx < 4); return data[idx]; }
    AGZ_FORCE_INLINE float operator[](size_t idx) const { AGZ_ASSERT(idx < 4); return data[idx]; }

    AGZ_FORCE_INLINE Self operator+(const Self &rhs) const { return Self(_mm_add_ps(d, rhs.d)); }
    AGZ_FORCE_INLINE Self operator-(const Self &rhs) const { return Self(_mm_sub_ps(d, rhs.d)); }
    AGZ_FORCE_INLINE Self operator*(const Self &rhs) const { return Self(_mm_mul_ps(d, rhs.d)); }
    AGZ_FORCE_INLINE Self operator/(const Self &rhs) const { return Self(_mm_div_ps(d, rhs.d)); }

    AGZ_FORCE_INLINE bool operator<(const Self &rhs) const { return _mm_movemask_ps(_mm_cmplt_ps(d, rhs.d)) == 0x0f; }
};

static_assert(sizeof(f32x4) == 16 && alignof(f32x4) == 16);

AGZ_FORCE_INLINE f32x4 Abs(const f32x4 &v) { return f32x4(_mm_and_ps(v.d, _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff)))); }

AGZ_FORCE_INLINE f32x4 Sqrt(const f32x4 &v) { return f32x4(_mm_sqrt_ps(v.d)); }

AGZ_FORCE_INLINE bool ApproxEq(const f32x4 &lhs, const f32x4 &rhs, float epsilon)
{
    return Abs(lhs - rhs) < f32x4(epsilon);
}

AGZ_NS_END(AGZ::Math)

#endif
