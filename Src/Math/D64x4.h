#pragma once

#include "../Misc/Common.h"
#include "../Misc/Config.h"

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

	D64x4()
		: d(_mm256_setzero_pd())
	{

	}

	explicit D64x4(Uninitialized_t)
	{

	}

	D64x4(double x, double y, double z, double w)
		: d(_mm256_set_pd(w, z, y, x))
	{

	}

	explicit D64x4(double v)
		: d(_mm256_set1_pd(v))
	{

	}

	explicit D64x4(double *data)
		: d(_mm256_load_pd(data))
	{

	}

	explicit D64x4(const __m256d other)
		: d(other)
	{

	}

	explicit D64x4(const Vec4d &v)
		: d(_mm256_load_pd(&v.x))
	{

	}

	D64x4(const Self &other)
		: d(other.d)
	{

	}

	Vec4d AsVec() const
	{
		return Vec4d(data);
	}

	explicit operator Vec4d() const
	{
		return AsVec();
	}

	Self &operator=(const Self &other)
	{
		d = other.d;
		return *this;
	}

	double &operator[](size_t idx) { AGZ_ASSERT(idx < 4); return data[idx]; }
	double operator[](size_t idx) const { AGZ_ASSERT(idx < 4); return data[idx]; }

	Self operator+(const Self &rhs) const { return Self(_mm256_add_pd(d, rhs.d)); }
	Self operator-(const Self &rhs) const { return Self(_mm256_sub_pd(d, rhs.d)); }
	Self operator*(const Self &rhs) const { return Self(_mm256_mul_pd(d, rhs.d)); }
	Self operator/(const Self &rhs) const { return Self(_mm256_div_pd(d, rhs.d)); }

	/**
	 * 是否每个元素都小于rhs
	 */
	bool ElemwiseLessThan(const Self &rhs) const { return _mm256_movemask_pd(_mm256_cmp_pd(d, rhs.d, 1)) == 0x0f; }
};

static_assert(sizeof(D64x4) == 32 && alignof(D64x4) == 32);

inline D64x4 Abs(D64x4 v)
{
	return D64x4(_mm256_and_pd(v.d, _mm256_castsi256_pd(_mm256_set1_epi64x(0x7fffffffffffffff))));
}

inline D64x4 Min(D64x4 lhs, D64x4 rhs)
{
	return D64x4(_mm256_min_pd(lhs.d, rhs.d));
}

inline D64x4 Max(D64x4 lhs, D64x4 rhs)
{
	return D64x4(_mm256_max_pd(lhs.d, rhs.d));
}

inline D64x4 Sqrt(D64x4 v)
{
	return D64x4(_mm256_sqrt_pd(v.d));
}

inline bool ApproxEq(D64x4 lhs, D64x4 rhs, double epsilon)
{
	return Abs(lhs - rhs).ElemwiseLessThan(D64x4(epsilon));
}

inline double Dot3(D64x4 lhs, D64x4 rhs)
{
	D64x4 r = lhs * rhs;
	return r.x + r.y + r.z;
}

inline double Dot(D64x4 lhs, D64x4 rhs)
{
	D64x4 r = lhs * rhs;
	return r.x + r.y + r.z + r.w;
}

} // namespace AGZ::Math

#endif
