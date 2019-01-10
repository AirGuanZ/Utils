#define SWIZZLE2(A, B)       _SWIZZLE2(A, B)
#define SWIZZLE3(A, B, C)    _SWIZZLE3(A, B, C)
#define SWIZZLE4(A, B, C, D) _SWIZZLE4(A, B, C, D)

#define _SWIZZLE2(A, B)       Vec2<T> A##B() const { return Vec2<T>(A, B); }
#define _SWIZZLE3(A, B, C)    Vec3<T> A##B##C() const { return Vec3<T>(A, B, C); }
#define _SWIZZLE4(A, B, C, D) Vec4<T> A##B##C##D() const { return Vec4<T>(A, B, C, D); }

SWIZZLE2(x, x)
SWIZZLE2(x, y)
SWIZZLE2(y, y)
SWIZZLE2(y, x)

#undef SWIZZLE2
#undef SWIZZLE3
#undef SWIZZLE4
#undef _SWIZZLE2
#undef _SWIZZLE3
#undef _SWIZZLE4
