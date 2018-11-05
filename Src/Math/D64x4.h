#pragma once

#include "../Misc/Common.h"
#include "../Misc/Config.h"

#if defined(AGZ_USE_AVX)

#include <immintrin.h>

#include "../Math/Vec4.h"

namespace AGZ::Math {

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

	AGZ_FORCEINLINE D64x4()
		: d(_mm256_setzero_pd())
	{

	}

	AGZ_FORCEINLINE explicit D64x4(Uninitialized_t)
	{

	}

	AGZ_FORCEINLINE D64x4(double x, double y, double z, double w)
		: d(_mm256_set_pd(w, z, y, x))
	{

	}

	AGZ_FORCEINLINE explicit D64x4(double v)
		: d(_mm256_set1_pd(v))
	{

	}

	AGZ_FORCEINLINE explicit D64x4(double *data)
		: d(_mm256_load_pd(data))
	{

	}

	AGZ_FORCEINLINE explicit D64x4(const __m256d other)
		: d(other)
	{

	}

	AGZ_FORCEINLINE explicit D64x4(const Vec4d &v)
		: d(_mm256_load_pd(&v.x))
	{

	}

	AGZ_FORCEINLINE D64x4(const Self &other)
		: d(other.d)
	{

	}

	AGZ_FORCEINLINE Vec4d AsVec() const
	{
		return Vec4d(data);
	}

	AGZ_FORCEINLINE explicit operator Vec4d() const
	{
		return AsVec();
	}

	AGZ_FORCEINLINE Self &operator=(const Self &other)
	{
		d = other.d;
		return *this;
	}

	AGZ_FORCEINLINE double &operator[](size_t idx) { AGZ_ASSERT(idx < 4); return data[idx]; }
	AGZ_FORCEINLINE double operator[](size_t idx) const { AGZ_ASSERT(idx < 4); return data[idx]; }

	AGZ_FORCEINLINE Self operator+(const Self &rhs) const { return Self(_mm256_add_pd(d, rhs.d)); }
	AGZ_FORCEINLINE Self operator-(const Self &rhs) const { return Self(_mm256_sub_pd(d, rhs.d)); }
	AGZ_FORCEINLINE Self operator*(const Self &rhs) const { return Self(_mm256_mul_pd(d, rhs.d)); }
	AGZ_FORCEINLINE Self operator/(const Self &rhs) const { return Self(_mm256_div_pd(d, rhs.d)); }

	bool ElemwiseLessThan(const Self &rhs) const { return _mm256_movemask_pd(_mm256_cmp_pd(d, rhs.d, 1)) == 0x0f; }
};

static_assert(sizeof(D64x4) == 32 && alignof(D64x4) == 32);

AGZ_FORCEINLINE D64x4 Abs(D64x4 v)
{
	return D64x4(_mm256_and_pd(v.d, _mm256_castsi256_pd(_mm256_set1_epi64x(0x7fffffffffffffff))));
}

AGZ_FORCEINLINE D64x4 Min(D64x4 lhs, D64x4 rhs)
{
	return D64x4(_mm256_min_pd(lhs.d, rhs.d));
}

AGZ_FORCEINLINE D64x4 Max(D64x4 lhs, D64x4 rhs)
{
	return D64x4(_mm256_max_pd(lhs.d, rhs.d));
}

AGZ_FORCEINLINE D64x4 Sqrt(D64x4 v)
{
	return D64x4(_mm256_sqrt_pd(v.d));
}

AGZ_FORCEINLINE bool ApproxEq(D64x4 lhs, D64x4 rhs, double epsilon)
{
	return Abs(lhs - rhs).ElemwiseLessThan(D64x4(epsilon));
}

AGZ_FORCEINLINE double Dot3(D64x4 lhs, D64x4 rhs)
{
	D64x4 r = lhs * rhs;
	return r.x + r.y + r.z;
}

AGZ_FORCEINLINE double Dot(D64x4 lhs, D64x4 rhs)
{
	D64x4 r = lhs * rhs;
	return r.x + r.y + r.z + r.w;
}

} // namespace AGZ::Math

#endif
