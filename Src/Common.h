#pragma once

#ifdef _MSC_VER
    #define AGZ_FORCE_INLINE __forceinline
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

#define AGZ_USE_SSE

AGZ_NS_END(AGZ)
