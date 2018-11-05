#pragma once

#include "../Misc/Common.h"
#include "Aggregate.h"

namespace AGZ {

namespace RangeAux
{
    template<typename F>
    struct AllRHS
    {
        template<typename R>
        static bool Eval(R &&range, F &&func)
        {
            for(auto &&v : range)
            {
                if(!func(std::forward<decltype(v)>(v)))
                    return false;
            }
            return true;
        }
    };
}

template<typename F>
bool All(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::AllRHS<
                remove_rcv_t<F>>, remove_rcv_t<F>>(
                    std::forward<F>(func));
}

} // namespace AGZ
