#pragma once

#include <utility>

#include "../Common.h"

AGZ_NS_BEG(AGZ::Buffer)

template<typename E>
void DefaultElemInitializer(E *buf)
{
    new(buf)E();
}

template<typename E>
class Buffer2D final
{
    size_t w_;
    size_t h_;
    E *d_;

    template<typename F>
    Buffer2D(size_t w, size_t h, F &&initer)
        : w_(w), h_(w)
    {
        size_t s = w * h;
        AGZ_ASSERT(s > 0);
        d_ = static_cast<E*>(AGZ_ALIGNED_MALLOC(s * sizeof(E), alignof(E)));
        if(!d_)
            throw std::bad_alloc();

        for(size_t i = 0; i < s; ++i)
            initer(d_ + i);
    }

    template<typename F>
    Buffer2D(CONS_FLAG_FROM_FN_t, size_t w, size_t h, F &&initer)
        : w_(w), h_(w)
    {
        size_t s = w * h;
        AGZ_ASSERT(s > 0);
        d_ = static_cast<E*>(AGZ_ALIGNED_MALLOC(s * sizeof(E), alignof(E)));
        if(!d_)
            throw std::bad_alloc();

        E *d = d_;
        for(size_t y = 0; y < h; ++y)
        {
            for(size_t x = 0; x < w; ++x)
                initer(x, y, d++);
        }
    }

    void Free()
    {
        AGZ_ASSERT(d_ != nullptr);
        size_t s = w_ * h_;
        for(size_t i = 0; i < s; ++i)
            (d_ + i)->~E();
        AGZ_ALIGNED_FREE(d_);
    }

public:

    using Elem = E;
    using Self = Buffer2D<E>;

    template<typename F = void(*)(E*)>
    static Self New(size_t w, size_t h, F &&initer = &DefaultElemInitializer<E>)
    {
        Self ret(w, h, initer);
        return std::move(ret);
    }

    template<typename F>
    static Self FromFn(size_t w, size_t h, F &&initer)
    {
        Self ret(FROM_FN, w, h, initer);
        return std::move(ret);
    }

    Buffer2D(Self &&moveFrom) noexcept
        : w_(moveFrom.w_), h_(moveFrom.h_), d_(moveFrom.d_)
    {
        moveFrom.w_ = moveFrom.h_ = 0;
        moveFrom.d_ = nullptr;
    }

    Self &operator=(Self &&moveFrom) noexcept
    {
        if(d_)
            Free();
        w_ = moveFrom.w_;
        h_ = moveFrom.h_;
        d_ = moveFrom.d_;
        moveFrom.w_ = moveFrom.h_ = 0;
        moveFrom.d_ = nullptr;
        return *this;
    }

    ~Buffer2D()
    {
        if(d_)
            Free();
    }
};

AGZ_NS_END(AGZ::Buffer)
