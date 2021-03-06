﻿#pragma once

#include <atomic>
#include <type_traits>

#include "../Misc/Common.h"
#include "../Alloc/Malloc.h"
#include "../Alloc/Alloc.h"

namespace AGZ {

/**
 * @brief 将任意类型封装为引用计数的写时复制类型，线程不安全
 */
template<typename T, typename Alloc = DefaultAllocator>
class COWObject
{
    using RefCounter = size_t;

    struct alignas(StaticMax(alignof(T), alignof(RefCounter))) Storage
    {
        T obj;
        RefCounter refs_;
    };

    Storage *storage_;

public:

    using Self   = COWObject<T, Alloc>; ///< 自身类型
    using Object = T;                   ///< 内部存储的对象类型

    /** 默认不存储任何对象 */
    COWObject() noexcept
        : storage_(nullptr)
    {
        
    }

    /**
     * @param args 转发给内部对象构造函数的参数
     * 
     * @exception std::bad_alloc 内存分配失败时抛出
     */
    template<typename...Args>
    explicit COWObject(Args&&...args)
    {
        storage_ = alloc_throw<Storage>(
            AGZ::aligned_alloc, alignof(Storage), sizeof(Storage));
        try
        {
            new(storage_) T(std::forward<Args>(args)...);
            storage_->refs_ = 1;
        }
        catch(...)
        {
            AGZ::aligned_free(storage_);
            throw;
        }
    }

    /** 共享copyFrom所持有的对象的所有权 */
    COWObject(const Self &copyFrom) noexcept
        : storage_(copyFrom.storage_)
    {
        if(storage_)
            ++storage_->refs_;
    }

    /** 攫取moveFrom所持有的所有权 */
    COWObject(Self &&moveFrom) noexcept
        : storage_(moveFrom.storage_)
    {
        moveFrom.storage_ = nullptr;
    }

    /** 释放持有的对象的所有权 */
    ~COWObject()
    {
        Release();
    }

    /** 放弃原本持有对象的所有权，共享copyFrom持有对象的所有权 */
    COWObject<T, Alloc> &operator=(const Self &copyFrom)
    {
        Release();
        storage_ = copyFrom.storage_;
        if(storage_)
            ++storage_->refs_;
        return *this;
    }
    
    /** 放弃原本持有对象的所有权，攫取moveFrom持有对象的所有权 */
    COWObject<T, Alloc> &operator=(Self &&moveFrom) noexcept
    {
        Release();
        storage_ = moveFrom.storage_;
        moveFrom.storage_ = nullptr;
        return *this;
    }

    /** 释放自己所持有的共享所有权，若自己是最后一个持有者，销毁内部对象 */
    void Release()
    {
        if(storage_ && !--storage_->refs_)
        {
            storage_->obj.~T();
            AGZ::aligned_free(storage_);
            storage_ = nullptr;
        }
    }

    /** 内部对象共享所有权的持有者数量 */
    RefCounter Refs() const noexcept
    {
        return storage_ ? storage_->refs_ : 0;
    }

    /** 是否持有某个对象的所有权 */
    bool IsAvailable() const noexcept
    {
        return storage_ != nullptr;
    }

    /** 是否持有某个对象的所有权 */
    operator bool() const noexcept
    {
        return IsAvailable();
    }

    /** 取得内部对象的常量引用 */
    const T &operator*() const noexcept
    {
        AGZ_ASSERT(storage_);
        return storage_->obj;
    }

    /** 将调用转发给内部对象 */
    const T *operator->() const noexcept
    {
        AGZ_ASSERT(storage_);
        return &storage_->obj;
    }

    /**
     * 取得内部对象的可变指针
     * 
     * @note 若内部对象持有者数量大于1，则该操作会将内部对象复制一份，并持有复制出的新对象
     */
    T *MutablePtr()
    {
        return &Mutable();
    }

    /**
     * 取得内部对象的可变引用
     * 
     * @note 若内部对象持有者数量大于1，则该操作会将内部对象复制一份，并持有复制出的新对象
     */
    T &Mutable()
    {
        AGZ_ASSERT(storage_);
        if(storage_->refs_ > 1)
        {
            auto newStorage_ = alloc_throw<Storage>(
                AGZ::aligned_alloc, alignof(Storage), sizeof(Storage));
            try
            {
                new(newStorage_) T(storage_->obj);
                newStorage_->refs_ = 1;
            }
            catch(...)
            {
                AGZ::aligned_free(newStorage_);
                throw;
            }
            --storage_->refs_;
            storage_ = newStorage_;
        }
        AGZ_ASSERT(storage_->refs_ == 1);
        return storage_->obj;
    }
};

} // namespace AGZ
