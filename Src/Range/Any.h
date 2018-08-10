#pragma once

#pragma once

#include "../Misc/Common.h"
#include "Aggregate.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<typename F>
    struct AnyRHS
    {
        template<typename R>
        static bool Eval(R &&range, F &&func)
        {
            for(auto &&v : range)
            {
                if(func(std::forward<decltype(v)>(v)))
                    return true;
            }
            return false;
        }
    };
}

template<typename F>
bool Any(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::AnyRHS<
                remove_rcv_t<F>>, remove_rcv_t<F>>(
                    std::forward<F>(func));
}

AGZ_NS_END(AGZ)
