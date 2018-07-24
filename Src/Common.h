#pragma once

#ifdef _MSC_VER

    #define AGZ_FORCE_INLINE __forceinline

    #include <cstdlib>
    #define AGZ_ALIGNED_MALLOC(S, A) _aligned_malloc((S), (A))
    #define AGZ_ALIGNED_FREE(P)   _aligned_free((P))

#elif defined __GNUC__
    #define AGZ_FORCE_INLINE __attribute__((always_inline))
#else
    #define AGZ_FORCE_INLINE inline
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

#define AGZ_USE_SSE2

AGZ_NS_END(AGZ)
