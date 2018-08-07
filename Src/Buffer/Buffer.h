#pragma once

#include <cstdlib>
#include <new>
#include <utility>

#include "../Misc/Common.h"
#include "../Misc/Malloc.h"

AGZ_NS_BEG(AGZ::Buf)

template<typename E>
E DefaultElementInitializer()
{
    return E();
}

template<typename E, typename N>
E DefaultElementTransformer(E &src)
{
    return N(src);
}

template<typename E, typename N>
E DefaultConstElementTransformer(const E &src)
{
    return N(src);
}

template<typename E>
class Buffer
{
    size_t s_;
    E *d_;

    void Alloc(size_t s)
    {
        AGZ_ASSERT(s > 0);
        d_ = alloc_throw<E>(::AGZ::aligned_alloc,
                            alignof(E), s * sizeof(E));
    }

    void Free()
    {
        AGZ_ASSERT(d_ != nullptr);
        for(size_t i = 0; i < s_; ++i)
            (d_ + i)->~E();
        ::AGZ::aligned_free(d_);
    }

public:

    using Elem = E;
    using Self = Buffer<E>;

    Buffer()
        : s_(0), d_(nullptr)
    {

    }

    template<typename F = E(*)()>
    explicit Buffer(size_t s, F &&initer = &DefaultElementInitializer)
        : s_(s)
    {
        Alloc(s);
        for(size_t i = 0; i < s; ++i)
            new(d_ + i) E(initer());
    }

    template<typename F>
    Buffer(CONS_FLAG_FROM_FN_t, size_t s, F &&initer)
        : s_(s)
    {
        Alloc(s);
        for(size_t i = 0; i < s; ++i)
            new(d_ + i) E(initer(i));
    }

    template<typename F = E(*)()>
    static Self New(size_t s, F &&initer = &DefaultElementInitializer)
    {
        Self ret(s, std::forward<F>(initer));
        return std::move(ret);
    }

    template<typename F>
    static Self FromFn(size_t s, F &&initer)
    {
        Self ret(FROM_FN, s, std::forward<F>(initer));
        return std::move(ret);
    }

    template<typename A, typename F = E(*)(const A&)>
    static Self FromConstOther(
        const Buffer<A> &transformFrom,
        F &&f = &DefaultConstElementTransformer)
    {
        return transformFrom.template Map<E, F>(std::forward<F>(f));
    }

    template<typename A, typename F = E(*)(A&)>
    static Self FromOther(Buffer<A> &transformFrom,
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

    Buffer(Self &&moveFrom) noexcept
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

    ~Buffer()
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

    bool IsAvailable() const
    {
        return d_ != nullptr;
    }

    E &operator()(size_t i)
    {
        AGZ_ASSERT(IsAvailable() && i < s_);
        return d_[i];
    }

    const E &operator()(size_t i) const
    {
        AGZ_ASSERT(IsAvailable()() && i < s_);
        return d_[i];
    }

    template<typename F>
    void Each(F &&f)
    {
        AGZ_ASSERT(IsAvailable());
        E *d = d_;
        for(size_t i = 0; i < s_; ++i)
            f(*d++);
    }

    template<typename F>
    void Each(F &&f) const
    {
        AGZ_ASSERT(IsAvailable());
        const E *d = d_;
        for(size_t i = 0; i < s_; ++i)
            f(*d++);
    }

    template<typename F>
    void EachIndex(F &&f)
    {
        AGZ_ASSERT(IsAvailable());
        E *d = d_;
        for(size_t i = 0; i < s_; ++i)
            f(i, *d++);
    }

    template<typename F>
    void EachIndex(F &&f) const
    {
        AGZ_ASSERT(IsAvailable());
        const E *d = d_;
        for(size_t i = 0; i < s_; ++i)
            f(i, *d++);
    }

    template<typename N, typename F = N(*)(E&)>
    Buffer<N> Map(F &&f = &DefaultElementTransformer<E, N>)
    {
        AGZ_ASSERT(IsAvailable());
        return Buffer<N>::FromFn(s_, [&](size_t i)
        {
            return f((*this)(i));
        });
    }

    template<typename N, typename F = N(*)(const E&)>
    Buffer<N> Map(F &&f = &DefaultElementTransformer<E, N>) const
    {
        AGZ_ASSERT(IsAvailable());
        return Buffer<N>::FromFn(s_, [&](size_t i)
        {
            return f((*this)(i), buf);
        });
    }

    template<typename A, typename F>
    A Foldl(const A &init, F &&f) const
    {
        A v = init;
        for(size_t i = 0; i < s_; ++i)
            v = f(v, d_[i]);
        return std::move(v);
    }
};

AGZ_NS_END(AGZ::Buf)
