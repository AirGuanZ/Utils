#pragma once

#include <functional>

#if defined(_MSC_VER)
#include <malloc.h>
#else
#include <cstdlib>
#endif

#include "../Misc/Common.h"

namespace AGZ {

// Aligned memory allocation

// MSVC doesn't implement std::aligned_alloc/free
#if defined(_MSC_VER)
    AGZ_FORCEINLINE void *aligned_alloc(size_t align, size_t size)
    {
        return ::_aligned_malloc(size, align);
    }
    AGZ_FORCEINLINE void aligned_free(void *ptr)
    {
        return ::_aligned_free(ptr);
    }
#else
    AGZ_FORCEINLINE void *aligned_alloc(size_t align, size_t size)
    {
        return std::aligned_alloc(align, size);
    }
    AGZ_FORCEINLINE void aligned_free(void *ptr)
    {
        std::free(ptr);
    }
#endif

// throw std::bad_alloc when !f(args...) == true
template<typename D, typename F, typename...Args>
AGZ_FORCEINLINE D *alloc_throw(F &&alloc_func, Args&&...args)
{
    auto ret = alloc_func(std::forward<Args>(args)...);
    if(!ret)
        throw std::bad_alloc();
    return static_cast<D*>(ret);
}

} // namespace AGZ
