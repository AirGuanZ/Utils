#pragma once

#ifdef _MSC_VER

#define AGZ_INLINE inline //__forceinline

#elif defined __GNUC__

#define AGZ_INLINE inline //__attribute__((always_inline))

#else

#define AGZ_INLINE inline

#endif

#ifdef _MSC_VER

#include <malloc.h>
#define AGZ_ALIGNED_MALLOC(S, A) _aligned_malloc((S), (A))
#define AGZ_ALIGNED_FREE(P)   _aligned_free((P))

#elif defined __GNUC__

#define AGZ_ALIGNED_MALLOC(S, A) _mm_malloc((A), (S))
#define AGZ_ALIGNED_FREE(P) _mm_free((P))

#else

#include <cstdlib>
#define AGZ_ALIGNED_MALLOC(S, A) std::malloc((S))
#define AGZ_ALIGNED_FREE(P) std::free((P))
#warning "Aligned malloc/free unimplemented, use std::malloc/free instead"

#endif

#define AGZ_NS_BEG(N) namespace N {
#define AGZ_NS_END(N) }

#include <cassert>
#define AGZ_ASSERT(X) assert(X)

#define AGZ_USE_SSE2

#ifndef __BYTE_ORDER__

#ifdef _MSC_VER

#define AGZ_LITTLE_ENDIAN

#else

// #define AGZ_BIG_ENDIAN
#define AGZ_LITTLE_ENDIAN

#endif

#endif

AGZ_NS_BEG(AGZ)

struct Uninitialized_t { };
inline Uninitialized_t UNINITIALIZED;

struct CONS_FLAG_FROM_FN_t { };
inline CONS_FLAG_FROM_FN_t FROM_FN;

struct Void_t { };

AGZ_NS_END(AGZ)
