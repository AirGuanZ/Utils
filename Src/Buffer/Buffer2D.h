#pragma once

#include <cstdlib>
#include <new>
#include <utility>

#include "../Misc/Common.h"
#include "Buffer.h"

AGZ_NS_BEG(AGZ::Buf)

template<typename E>
class Buffer2D
{
    size_t w_;
    size_t h_;
    E *d_;

    void Alloc(size_t num)
    {
        AGZ_ASSERT(num > 0);
        d_ = static_cast<E*>(std::aligned_alloc(alignof(E), num * sizeof(E)));
        if(!d_)
            throw std::bad_alloc();
    }

    void Free()
    {
        AGZ_ASSERT(d_ != nullptr);
        size_t s = w_ * h_;
        for(size_t i = 0; i < s; ++i)
            (d_ + i)->~E();
        std::free(d_);
    }

    size_t GetIdx(size_t x, size_t y) const
    {
        AGZ_ASSERT(x < w_ && y < h_);
        return y * w_ + x;
    }

public:

    using Elem = E;
    using Self = Buffer2D<E>;

    Buffer2D()
        : w_(0), h_(0), d_(nullptr)
    {

    }

    template<typename F = void(*)(E*)>
    Buffer2D(size_t w, size_t h, F &&initer = &DefaultElementInitializer)
        : w_(w), h_(h)
    {
        size_t s = w * h;
        Alloc(s);

        for(size_t i = 0; i < s; ++i)
            initer(d_ + i);
    }

    template<typename F>
    Buffer2D(CONS_FLAG_FROM_FN_t, size_t w, size_t h, F &&initer)
        : w_(w), h_(h)
    {
        Alloc(w * h);
        E *d = d_;
        for(size_t y = 0; y < h; ++y)
        {
            for(size_t x = 0; x < w; ++x)
                initer(x, y, d++);
        }
    }

    template<typename F = void(*)(E*)>
    static Self New(size_t w, size_t h,
                    F &&initer = &DefaultElementInitializer<E>)
    {
        Self ret(w, h, std::forward<F>(initer));
        return std::move(ret);
    }

    template<typename F>
    static Self FromFn(size_t w, size_t h, F &&initer)
    {
        Self ret(FROM_FN, w, h, std::forward<F>(initer));
        return std::move(ret);
    }

    template<typename A, typename F = void(*)(const A*, E*)>
    static Self FromConstOther(
        const Buffer2D<A> &transformFrom,
        F &&f = &DefaultConstElementTransformer)
    {
        return transformFrom.template Map<E, F>(std::forward<F>(f));
    }

    template<typename A, typename F = void(*)(A*, E*)>
    static Self FromOther(Buffer2D<A> &transformFrom,
                                           F &&f = &DefaultElementTransformer)
    {
        return transformFrom.template Map<E, F>(std::forward<F>(f));
    }

    Buffer2D(const Self &copyFrom)
    {
        w_ = copyFrom.w_;
        h_ = copyFrom.h_;

        if(!copyFrom.IsAvailable())
        {
            d_ = nullptr;
            return;
        }

        size_t s = w_ * h_;
        Alloc(s);

        for(size_t i = 0; i < s; ++i)
            new (d_ + i) E(copyFrom.d_[i]);
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

    Self &operator=(const Self &copyFrom)
    {
        if(d_)
            Free();
        w_ = copyFrom.w_;
        h_ = copyFrom.h_;

        if(!copyFrom.IsAvailable())
        {
            d_ = nullptr;
            return *this;
        }

        size_t s = w_ * h_;
        Alloc(s);

        for(size_t i = 0; i < s; ++i)
            new (d_ + i) E(copyFrom.d_[i]);

        return *this;
    }

    ~Buffer2D()
    {
        if(IsAvailable())
            Free();
    }

    void Destroy()
    {
        AGZ_ASSERT(IsAvailable());
        Free();
        w_ = h_ = 0;
        d_ = nullptr;
    }

    bool IsAvailable() const
    {
        return d_ != nullptr;
    }

    E &operator()(size_t x, size_t y)
    {
        AGZ_ASSERT(IsAvailable());
        return d_[GetIdx(x, y)];
    }

    const E &operator()(size_t x, size_t y) const
    {
        AGZ_ASSERT(IsAvailable());
        return d_[GetIdx(x, y)];
    }

    template<typename F>
    void ForAll(F &&f)
    {
        AGZ_ASSERT(IsAvailable());
        E *d = d_;
        for(size_t y = 0; y < h_; ++y)
        {
            for(size_t x = 0; x < w_; ++x)
                f(x, y, d++);
        }
    }

    template<typename F>
    void ForAll(F &&f) const
    {
        AGZ_ASSERT(IsAvailable());
        const E *d = d_;
        for(size_t y = 0; y < h_; ++y)
        {
            for(size_t x = 0; x < w_; ++x)
                f(x, y, d++);
        }
    }

    template<typename N, typename F = void(*)(E*, N*)>
    Buffer2D<N> Map(F &&f = &DefaultElementTransformer<E, N>)
    {
        AGZ_ASSERT(IsAvailable());
        return Buffer2D<N>::FromFn(w_, h_,
            [&](size_t x, size_t y, N *buf)
        {
            f(&(*this)(x, y), buf);
        });
    }

    template<typename N, typename F = void(*)(const E*, N*)>
    Buffer2D<N> Map(F &&f = &DefaultElementTransformer<E, N>) const
    {
        AGZ_ASSERT(IsAvailable());
        return Buffer2D<N>::FromFn(w_, h_,
            [&](size_t x, size_t y, N *buf)
        {
            f(&(*this)(x, y), buf);
        });
    }
};

AGZ_NS_END(AGZ::Buf)
