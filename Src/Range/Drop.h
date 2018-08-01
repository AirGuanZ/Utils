#pragma once

#include "../Misc/Common.h"
#include "Range/Iterator.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<typename R>
    class DropImpl
    {
        R range_;
        typename R::Iterator beg_;

    public:

        using Iterator = typename R::Iterator;

        DropImpl(R range, size_t n)
            : range_(std::move(range))
        {
            beg_ = AdvanceTo(std::begin(range_), std::end(range_),
                        static_cast<typename R::Iterator::difference_type>(n));
        }

        Iterator begin() const { return beg_; }
        Iterator end() const { return std::end(range_); }
    };

    template<typename R, typename F>
    class DropWhileImpl
    {
        R range_;
        typename R::Iterator beg_;

    public:

        using Iterator = typename R::Iterator;

        DropWhileImpl(R range, F &&func)
            : range_(std::move(range))
        {
            Iterator end = std::end(range_);
            beg_ = std::begin(range_);
            while(beg_ != end && func(*beg_))
                ++beg_;
        }

        Iterator begin() const { return beg_; }
        Iterator end() const { return std::end(range_); }
    };

    struct DropTrait
    {
        template<typename R>
        using Impl = DropImpl<R>;
    };

    struct DropWhileTrait
    {
        template<typename R>
        using Impl = DropWhileImpl<R>;
    };
}

inline auto Drop(size_t n)
{
    return RangeAux::TransformWrapper<
            RangeAux::DropTrait, size_t>(n);
}

template<typename F>
auto DropWhile(F &&func)
{
    return RangeAux::TransformWrapper<
            RangeAux::DropWhileTrait, remove_rcv_t<F>>(
                std::forward<F>(func));
}

AGZ_NS_END(AGZ)
