#pragma once

#include "../Common.h"

#ifdef AGZ_USE_SSE2

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

    f32x4()
        : d(_mm_setzero_ps())
    {
        
    }

    explicit f32x4(Uninitialized_t)
    {
        
    }

    f32x4(float x, float y, float z, float w)
        : d(_mm_set_ps(w, z, y, x))
    {
        
    }

    explicit f32x4(float v)
        : d(_mm_set_ps1(v))
    {
        
    }

    explicit f32x4(const float *data)
        : d(_mm_load_ps(data))
    {
        
    }

    explicit f32x4(const __m128 other)
        : d(other)
    {
        
    }

    explicit f32x4(const Vec4f &v)
        : d(_mm_load_ps(&v.x))
    {

    }

    f32x4(const Self &other)
        : d(other.d)
    {

    }

    Vec4f AsVec() const
    {
        return Vec4f(data);
    }

    Self &operator=(const Self &other)
    {
        m = other.m;
        return *this;
    }

    float &operator[](size_t idx) { AGZ_ASSERT(idx < 4); return data[idx]; }
    float operator[](size_t idx) const { AGZ_ASSERT(idx < 4); return data[idx]; }

    Self operator+(const Self &rhs) const { return Self(_mm_add_ps(d, rhs.d)); }
    Self operator-(const Self &rhs) const { return Self(_mm_sub_ps(d, rhs.d)); }
    Self operator*(const Self &rhs) const { return Self(_mm_mul_ps(d, rhs.d)); }
    Self operator/(const Self &rhs) const { return Self(_mm_div_ps(d, rhs.d)); }

    bool operator<(const Self &rhs) const { return _mm_movemask_ps(_mm_cmplt_ps(d, rhs.d)) == 0x0f; }
};

static_assert(sizeof(f32x4) == 16 && alignof(f32x4) == 16);

AGZ_INLINE f32x4 Abs(const f32x4 &v) { return f32x4(_mm_and_ps(v.d, _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff)))); }

AGZ_INLINE f32x4 Sqrt(const f32x4 &v) { return f32x4(_mm_sqrt_ps(v.d)); }

AGZ_INLINE bool ApproxEq(const f32x4 &lhs, const f32x4 &rhs, float epsilon)
{
    return Abs(lhs - rhs) < f32x4(epsilon);
}

namespace F32X4
{
    inline const f32x4 ZERO = f32x4(0.0f);
    inline const f32x4 ONE  = f32x4(1.0f);

    inline const f32x4 UNIT_X = f32x4(1.0f, 0.0f, 0.0f, 0.0f);
    inline const f32x4 UNIT_Y = f32x4(0.0f, 1.0f, 0.0f, 0.0f);
    inline const f32x4 UNIT_Z = f32x4(0.0f, 0.0f, 1.0f, 0.0f);
    inline const f32x4 UNIT_W = f32x4(0.0f, 0.0f, 0.0f, 1.0f);
}

AGZ_NS_END(AGZ::Math)

#endif
