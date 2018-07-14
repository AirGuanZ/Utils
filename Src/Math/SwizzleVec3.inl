#define SWIZZLE2(A, B)       _SWIZZLE2(A, B)
#define SWIZZLE3(A, B, C)    _SWIZZLE3(A, B, C)
#define SWIZZLE4(A, B, C, D) _SWIZZLE4(A, B, C, D)

#define _SWIZZLE2(A, B)       AGZ_FORCE_INLINE Vec2<T> A##B() const { return Vec2<T>(A, B); }
#define _SWIZZLE3(A, B, C)    AGZ_FORCE_INLINE Vec3<T> A##B##C() const { return Vec3<T>(A, B, C); }
#define _SWIZZLE4(A, B, C, D) AGZ_FORCE_INLINE Vec4<T> A##B##C##D() const { return Vec4<T>(A, B, C, D); }

SWIZZLE2(x, x)
SWIZZLE2(x, y)
SWIZZLE2(x, z)

SWIZZLE2(y, x)
SWIZZLE2(y, y)
SWIZZLE2(y, z)

SWIZZLE2(z, x)
SWIZZLE2(z, y)
SWIZZLE2(z, z)

SWIZZLE3(x, x, x)
SWIZZLE3(x, x, y)
SWIZZLE3(x, x, z)
SWIZZLE3(x, y, x)
SWIZZLE3(x, y, y)
SWIZZLE3(x, y, z)
SWIZZLE3(x, z, x)
SWIZZLE3(x, z, y)
SWIZZLE3(x, z, z)

SWIZZLE3(y, x, x)
SWIZZLE3(y, x, y)
SWIZZLE3(y, x, z)
SWIZZLE3(y, y, x)
SWIZZLE3(y, y, y)
SWIZZLE3(y, y, z)
SWIZZLE3(y, z, x)
SWIZZLE3(y, z, y)
SWIZZLE3(y, z, z)

SWIZZLE3(z, x, x)
SWIZZLE3(z, x, y)
SWIZZLE3(z, x, z)
SWIZZLE3(z, y, x)
SWIZZLE3(z, y, y)
SWIZZLE3(z, y, z)
SWIZZLE3(z, z, x)
SWIZZLE3(z, z, y)
SWIZZLE3(z, z, z)

#undef SWIZZLE2
#undef SWIZZLE3
#undef SWIZZLE4
#undef _SWIZZLE2
#undef _SWIZZLE3
#undef _SWIZZLE4
