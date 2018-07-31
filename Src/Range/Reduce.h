#pragma once

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<typename I, typename F>
    struct ReduceRHS { I init; F func; }
}

template<typename I, typename F>
auto Reduce(I init, F func)
{
    return RangeAux::ReduceRHS{ std::move(init), std::move(func) };
}

template<typename R, typename I, typename F>
auto operator|(R &&range, RangeAux::ReduceRHS<I, F> &&rhs)
{
    I ret = rhs.init;
    for(auto &val : range)
        ret = rhs.func(ret, val);
    return ret;
}

AGZ_NS_END(AGZ)
