#pragma once

#include <tuple>
#include <utility>

#include "../Misc/Common.h"

namespace AGZ {

namespace RangeAux
{
    template<typename RHS, typename...Args>
    struct AggregateWrapper
    {
        std::tuple<Args...> args;

        explicit AggregateWrapper(Args&&...args)
            : args(std::forward<Args>(args)...)
        {

        }

        template<typename R>
        auto Eval(R &&range)
        {
#ifdef AGZ_CC_GCC
            return std::apply<decltype(&RHS::template Eval<R>)>(
                                        &RHS::template Eval<R>,
                                        std::tuple_cat(std::make_tuple<R>(
                                            std::forward<R>(range)), args));
#else
            return std::apply(&RHS::template Eval<R>,
                              std::tuple_cat(
                                  std::make_tuple<R>(std::forward<R>(range)),
                                  args));
#endif
        }
    };

    template<typename R, typename RHS, typename...Args>
    auto operator|(R &&range, AggregateWrapper<RHS, Args...> &&opr)
    {
        return opr.Eval(std::forward<R>(range));
    }
}

} // namespace AGZ
