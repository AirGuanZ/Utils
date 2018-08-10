#pragma once

#include "../Misc/Common.h"
#include "Aggregate.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<typename F>
    struct EachRHS
    {
        template<typename R>
        static auto &&Eval(R &&range, F &&func)
        {
            for(auto &&v : range)
                func(std::forward<decltype(v)>(v));
            return std::forward<R>(range);
        }
    };

    template<typename F>
    struct EachIndexRHS
    {
        template<typename R>
        static auto &&Eval(R &&range, F &&func)
        {
            size_t i = 0;
            for(auto &&v : range)
                func(std::forward<decltype(v)>(v), i++);
            return std::forward<R>(range);
        }
    };
}

template<typename F>
auto Each(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::EachRHS<
                remove_rcv_t<F>>, remove_rcv_t<F>>(
                    std::forward<F>(func));
}

template<typename F>
auto EachIndex(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::EachIndexRHS<
                remove_rcv_t<F>>, remove_rcv_t<F>>(
                    std::forward<F>(func));
}

AGZ_NS_END(AGZ)
