#pragma once

#include "../Misc/Common.h"
#include "Aggregate.h"

namespace AGZ {

namespace RangeAux
{
    template<typename I, typename F>
    struct ReduceRHS
    {
        template<typename R>
        static remove_rcv_t<I> Eval(R &&range, I init, const F &func)
        {
            remove_rcv_t<I> ret = init;
            for(auto &&val : range)
                ret = func(ret, std::forward<decltype(val)>(val));
            return ret;
        }
    };
}

/**
 * @brief Reduce是干嘛的还需要写注释吗？
 */
template<typename I, typename F>
auto Reduce(I &&init, F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::ReduceRHS<
                I, F>, I, F>(std::forward<I>(init), std::forward<F>(func));
}

} // namespace AGZ
