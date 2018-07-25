#pragma once

#ifdef _MSC_VER
#define AGZ_FORCE_INLINE __forceinline
#elif defined __GNUC__
#define AGZ_FORCE_INLINE __attribute__((always_inline))
#else
#define AGZ_FORCE_INLINE inline
#endif

#ifdef _MSC_VER
#include <malloc.h>
#define AGZ_ALIGNED_MALLOC(S, A) _aligned_malloc((S), (A))
#define AGZ_ALIGNED_FREE(P)   _aligned_free((P))
#elif defined __GNUC__

#else
#include <cstdlib>
#define AGZ_ALIGNED_MALLOC(S, A) std::malloc((S))
#define AGZ_ALIGNED_FREE(P) std::free((P))
#warning "Aligned malloc/free unimplemented, use "
#endif

#define AGZ_NS_BEG(N) namespace N {
#define AGZ_NS_END(N) }

#include <cassert>
#define AGZ_ASSERT(X) assert(X)

AGZ_NS_BEG(AGZ)

struct Uninitialized_t { };
inline Uninitialized_t UNINITIALIZED;

struct CONS_FLAG_FROM_FN_t { };
inline CONS_FLAG_FROM_FN_t FROM_FN;

struct Void_t { };

[[noreturn]] void unreachable() noexcept
{
#ifdef _MSC_VER
    __assume(0);
#elif defined (__GNUC__)
    __builtin_unreachable();
#else
    std::terminate();
#endif
}

#define AGZ_USE_SSE2

AGZ_NS_END(AGZ)
