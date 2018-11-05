#pragma once

#include <cstdlib>

#include "Malloc.h"

namespace AGZ {

/*
    concept Allocator
    {
        // Throw bad_alloc if failed
        static void *Malloc(size_t size);
        static void *Malloc(size_t size, size_t align);

        static void Free(void *ptr);
        static void FreeAligned(void *ptr);
    }
*/

/**
 * @brief 基于C runtime的堆内存分配器
 * 
 * 主要用来作为其他更高层次的内存管理类的模板参数
 */
class CRTAllocator
{
public:

	/**
	 * 以系统默认对齐方式申请一块内存
	 * 
	 * @param size 申请的内存块的字节数
	 * @return 指向内存块首字节的指针
	 * 
	 * @exception std::bad_alloc
	 */
    static void *Malloc(size_t size)
    {
        return alloc_throw<void>(std::malloc, size);
    }

	/**
	 * 以指定的对齐值申请一块内存
	 * 
	 * @param size 申请的内存块的字节数
	 * @param align 申请的内存块的对齐值，应为2的整数次幂
	 * @return 指向内存块首字节的指针
	 * 
	 * @exception std::bad_alloc
	 */
    static void *Malloc(size_t size, size_t align)
    {
        return alloc_throw<void>(AGZ::aligned_alloc, align, size);
    }

	/**
	 * 释放一块以默认对齐方式申请的内存块
	 * 
	 * @param ptr 待释放内存块的首字节地址
	 */
    static void Free(void *ptr)
    {
        std::free(ptr);
    }

	/**
	 * 释放一块以指定对齐方式申请的内存块
	 * 
	 * @param ptr 待释放内存块的首字节地址
	 */
    static void FreeAligned(void *ptr)
    {
        AGZ::aligned_free(ptr);
    }
};

/**
 * 默认内存分配器
 */
using DefaultAllocator = CRTAllocator;

} // namespace AGZ
