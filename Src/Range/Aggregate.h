#pragma once

#include <iterator>
#include <tuple>
#include <utility>

#include "../Misc/Common.h"
#include "Iterator.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<typename RHS, typename...Args>
    struct AggregateWrapper
    {
        std::tuple<Args...> args;

        // TODO: move?
        explicit AggregateWrapper(Args&&...args)
            : args(std::forward<Args>(args)...)
        {

        }

        template<typename R>
        auto Eval(R &&range)
        {
            return std::apply(&RHS::template Eval<R>,
                              std::tuple_cat(
                                  std::make_tuple<R>(std::forward<R>(range)),
                                  args));
        }
    };
}

template<typename R, typename RHS, typename...Args>
auto operator|(R &&range, RangeAux::AggregateWrapper<RHS, Args...> &&opr)
{
    return opr.Eval(std::forward<R>(range));
}

AGZ_NS_END(AGZ)
