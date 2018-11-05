#pragma once

#include <atomic>
#include <type_traits>

#include "../Misc/Common.h"
#include "../Alloc/Malloc.h"
#include "../Alloc/Alloc.h"

namespace AGZ {

template<typename T,
         typename Alloc = DefaultAllocator,
         std::enable_if_t<Alloc::AnyAlign, int> = 0>
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

    using Self   = COWObject<T, Alloc>;
    using Object = T;

    COWObject()
        : storage_(nullptr)
    {
        
    }

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

    COWObject(const Self &copyFrom)
        : storage_(copyFrom.storage_)
    {
        if(storage_)
            ++storage_->refs_;
    }

    COWObject(Self &&moveFrom) noexcept
        : storage_(moveFrom.storage_)
    {
        moveFrom.storage_ = nullptr;
    }

    ~COWObject()
    {
        Release();
    }

    Self &operator=(const Self &copyFrom)
    {
        Release();
        storage_ = copyFrom.storage_;
        if(storage_)
            ++storage_->refs_;
        return *this;
    }

    Self &operator=(Self &&moveFrom)
    {
        Release();
        storage_ = moveFrom.storage_;
        moveFrom.storage_ = nullptr;
        return *this;
    }

    void Release()
    {
        if(storage_ && !--storage_->refs_)
        {
            storage_->obj.~T();
            AGZ::aligned_free(storage_);
            storage_ = nullptr;
        }
    }

    RefCounter Refs() const
    {
        return storage_ ? storage_->refs_ : 0;
    }

    bool IsAvailable() const
    {
        return storage_ != nullptr;
    }

    operator bool() const
    {
        return IsAvailable();
    }

    const T &operator*() const
    {
        AGZ_ASSERT(storage_);
        return storage_->obj;
    }

    const T *operator->() const
    {
        AGZ_ASSERT(storage_);
        return &storage_->obj;
    }

    T *MutablePtr()
    {
        return &Mutable();
    }

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
