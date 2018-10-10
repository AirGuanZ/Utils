#pragma once

#include "../Misc/Common.h"
#include "../Misc/Exception.h"
#include "../Misc/Uncopiable.h"

AGZ_NS_BEG(AGZ)

class ArbitaryArena : Uncopiable
{
public:

    virtual ~ArbitaryArena() = default;

    virtual void *Alloc(size_t size) = 0;

    virtual void Free(void *ptr) = 0;

    virtual void FreeAll() = 0;
};

template<typename T>
class FixedArena : Uncopiable
{
public:

    using AllocType = T;

    virtual ~FixedArena() = default;

    template<typename...Args>
    T *New(Args&&...args)
    {
        T *ret = this->Alloc();
        if constexpr(noexcept(T(std::forward<Args>(args)...)))
        {
            try
            {
                return new(ret) T(std::forward<Args>(args)...);
            }
            catch(...)
            {
                this->Free(ret);
                throw;
            }
        }
        else
            return new(ret) T(std::forward<Args>(args)...);
    }

    void Delete(T *ptr)
    {
        ptr->~T();
        this->Free(ptr);
    }

    virtual void DeleteAll()
    {
        throw UnreachableException("Default FixedArena::DeleteAll unimplemented");
    }

protected:

    virtual T *Alloc() = 0;

    virtual void Free(T *ptr) = 0;
};

AGZ_NS_END(AGZ)
