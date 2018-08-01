#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

#include "../Misc/Common.h"
#include "Transform.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<typename R>
    class TakeImpl
    {
        R range_;
        typename R::Iterator end_;

    public:

        using Iterator = typename R::Iterator;

        TakeImpl(R range, typename Iterator::difference_type num)
            : range_(std::move(range))
        {
            end_ = AdvanceTo(std::begin(range_), std::end(range_), num);
        }

        Iterator begin() const
        {
            return std::begin(range_);
        }

        Iterator end() const
        {
            return end_;
        }
    };

    template<typename R, typename F>
    class TakeWhileImpl
    {
        R range_;
        typename R::Iterator end_;

    public:

        using Iterator = typename R::Iterator;

        TakeWhileImpl(R range, F &&func)
            : range_(std::move(range))
        {
            Iterator beg = std::begin(range_), end = std::end(range_);
            while(beg != end && func(*beg))
                ++beg;
            end_ = beg;
        }

        Iterator begin() const
        {
            return std::begin(range_);
        }

        Iterator end() const
        {
            return end_;
        }
    };

    struct TakeTrait
    {
        template<typename R>
        using Impl = TakeImpl<R>;
    };

    template<typename F>
    struct TakeWhileTrait
    {
        template<typename R>
        using Impl = TakeWhileImpl<R, F>;
    };
}

inline auto Take(size_t n)
{
    return RangeAux::TransformWrapper<
        RangeAux::TakeTrait, size_t>(std::move(n));
}

template<typename F>
auto TakeWhile(F func)
{
    return RangeAux::TransformWrapper<
            RangeAux::TakeWhileTrait<remove_rcv_t<F>>,
            remove_rcv_t<F>>(
                std::move(func));
}

AGZ_NS_END(AGZ)
