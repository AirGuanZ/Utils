#pragma once

/**
 * @file Alloc/Malloc.h
 * @brief 一些常用的内存分配辅助函数
 */

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

    /*
     * @brief 按指定对齐方式分配一块内存，align应为2的整数次幂
     */
    AGZ_FORCEINLINE void *aligned_alloc(size_t align, size_t size)
    {
        return ::_aligned_malloc(size, align);
    }

    /**
     * @brief 释放用aligned_alloc分配的存储
     */
    AGZ_FORCEINLINE void aligned_free(void *ptr)
    {
        return ::_aligned_free(ptr);
    }
#else
    /*
     * @brief 按指定对齐方式分配一块内存，align应为2的整数次幂
     */
    AGZ_FORCEINLINE void *aligned_alloc(size_t align, size_t size)
    {
        return std::aligned_alloc(align, size);
    }

    /**
     * @brief 释放用aligned_alloc分配的存储
     */
    AGZ_FORCEINLINE void aligned_free(void *ptr)
    {
        std::free(ptr);
    }
#endif

/**
 * @brief 若给定的内存分配函数返回null，则抛出std::bad_alloc，否则按指定类型返回分配结果
 */
template<typename D, typename F, typename...Args>
AGZ_FORCEINLINE D *alloc_throw(F &&alloc_func, Args&&...args)
{
    auto ret = alloc_func(std::forward<Args>(args)...);
    if(!ret)
        throw std::bad_alloc();
    return static_cast<D*>(ret);
}

} // namespace AGZ
