#pragma once

#include "../Misc/Common.h"
#include "Aggregate.h"

AGZ_NS_BEG(AGZ)

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
    return RangeAux::AggregateWrapper<RangeAux::AllRHS<F>, F>(
        std::forward<F>(func));
}

AGZ_NS_END(AGZ)
