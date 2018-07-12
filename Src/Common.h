#pragma once

#ifdef _MSC_VER
    #define AGZ_FORCE_INLINE __forceinline
#else
    #define AGZ_FORCE_INLINE inline
#endif

#include <cassert>
#define AGZ_ASSERT(X) assert(X)
