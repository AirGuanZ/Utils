#pragma once

#include "../Common.h"

AGZ_NS_BEG(AGZ::Buf)

template<typename E>
AGZ_FORCE_INLINE void DefaultElementInitializer(E *buf)
{
    new (buf) E();
}

template<typename E, typename N>
AGZ_FORCE_INLINE void DefaultElementTransformer(E *src, N *dst)
{
    new (dst) N(*src);
}

template<typename E, typename N>
AGZ_FORCE_INLINE void DefaultConstElementTransformer(const E *src, N *dst)
{
    new (dst) N(*src);
}

template<typename E>
class Buffer
{
    size_t s_;
    E *d_;

    void Alloc(size_t s)
    {
        AGZ_ASSERT(s > 0);
        d_ = static_cast<E*>(AGZ_ALIGNED_MALLOC(
            s * sizeof(E), alignof(E)));
        if(!d_)
            throw std::bad_alloc();
    }

    void Free()
    {
        AGZ_ASSERT(d_ != nullptr);
        for(size_t i = 0; i < s_; ++i)
            (d_ + i)->~E();
        AGZ_ALIGNED_FREE(d_);
    }

public:

    using Elem = E;
    using Self = Buffer<E>;

    AGZ_FORCE_INLINE Buffer()
        : s_(0), d_(nullptr)
    {

    }

    template<typename F = void(*)(E*)>
    explicit Buffer(size_t s, F &&initer = &DefaultElementInitializer)
        : s_(s)
    {
        Alloc(s);
        for(size_t i = 0; i < s; ++i)
            initer(d_ + i);
    }

    template<typename F>
    Buffer(CONS_FLAG_FROM_FN_t, size_t s, F &&initer)
        : s_(s)
    {
        Alloc(s);
        for(size_t i = 0; i < s; ++i)
            initer(i, d_ + i);
    }

    template<typename F = void(*)(E*)>
    static AGZ_FORCE_INLINE Self New(size_t s,
                                     F &&initer = &DefaultElementInitializer)
    {
        Self ret(s, std::forward<F>(initer));
        return std::move(ret);
    }

    template<typename F>
    static AGZ_FORCE_INLINE Self FromFn(size_t s, F &&initer)
    {
        Self ret(FROM_FN, s, std::forward<F>(initer));
        return std::move(ret);
    }

    template<typename A, typename F = void(*)(const A*, E*)>
    static AGZ_FORCE_INLINE Self FromConstOther(const Buffer<A> &transformFrom,
                                                F &&f = &DefaultConstElementTransformer)
    {
        return transformFrom.template Map<E, F>(std::forward<F>(f));
    }

    template<typename A, typename F = void(*)(A*, E*)>
    static AGZ_FORCE_INLINE Self FromOther(Buffer<A> &transformFrom,
                                           F &&f = &DefaultElementTransformer)
    {
        return transformFrom.template Map<E, F>(std::forward<F>(f));
    }

    Buffer(const Self &copyFrom)
    {
        s_ = copyFrom.s_;

        if(!copyFrom.IsAvailable())
        {
            d_ = nullptr;
            return;
        }

        Alloc(s_);
        for(size_t i = 0; i < s_; ++i)
            new (d_ + i) E(copyFrom.d_[i]);
    }

    AGZ_FORCE_INLINE Buffer(Self &&moveFrom) noexcept
         : s_(moveFrom.s_), d_(moveFrom.d_)
    {
        moveFrom.s_ = 0;
        moveFrom.d_ = nullptr;
    }

    Self &operator=(Self &&moveFrom) noexcept
    {
        if(d_)
            Free();
        s_ = moveFrom.s_;
        d_ = moveFrom.d_;
        moveFrom.s_ = 0;
        moveFrom.d_ = nullptr;
        return *this;
    }

    Self &operator=(const Self &copyFrom)
    {
        if(d_)
            Free();
        s_ = copyFrom.s_;

        if(!copyFrom.IsAvailable())
        {
            d_ = nullptr;
            return *this;
        }

        Alloc(s_);
        for(size_t i = 0; i < s_; ++i)
            new (d_ + i) E(copyFrom.d_[i]);

        return *this;
    }

    AGZ_FORCE_INLINE ~Buffer()
    {
        if(IsAvailable())
            Free();
    }

    void Destroy()
    {
        AGZ_ASSERT(IsAvailable());
        Free();
        s_ = 0;
        d_ = nullptr;
    }

    AGZ_FORCE_INLINE bool IsAvailable() const
    {
        return d_ != nullptr;
    }

    AGZ_FORCE_INLINE E &operator()(size_t i)
    {
        AGZ_ASSERT(IsAvailable() && i < s_);
        return d_[i];
    }

    AGZ_FORCE_INLINE const E &operator()(size_t i) const
    {
        AGZ_ASSERT(IsAvailable()() && i < s_);
        return d_[i];
    }

    template<typename F>
    AGZ_FORCE_INLINE void ForAll(F &&f)
    {
        AGZ_ASSERT(IsAvailable());
        E *d = d_;
        for(size_t i = 0; i < s_; ++i)
            f(i, d++);
    }

    template<typename F>
    AGZ_FORCE_INLINE void ForAll(F &&f) const
    {
        AGZ_ASSERT(IsAvailable());
        const E *d = d_;
        for(size_t i = 0; i < s_; ++i)
            f(i, d++);
    }

    template<typename N, typename F = void(*)(E*, N*)>
    AGZ_FORCE_INLINE Buffer<N> Map(F &&f = &DefaultElementTransformer<E, N>)
    {
        AGZ_ASSERT(IsAvailable());
        return Buffer<N>::FromFn(s_,
            [&](size_t i, N *buf)
        {
            f(&(*this)(i), buf);
        });
    }

    template<typename N, typename F = void(*)(const E*, N*)>
    AGZ_FORCE_INLINE Buffer<N> Map(F &&f = &DefaultElementTransformer<E, N>) const
    {
        AGZ_ASSERT(IsAvailable());
        return Buffer<N>::FromFn(s_,
            [&](size_t i, N *buf)
        {
            f(&(*this)(i), buf);
        });
    }

    template<typename A, typename F>
    AGZ_FORCE_INLINE A Foldl(const A &init, F &&f) const
    {
        A v = init;
        for(size_t i = 0; i < s_; ++i)
            v = f(v, d_[i]);
        return std::move(v);
    }
};

AGZ_NS_END(AGZ::Buf)
