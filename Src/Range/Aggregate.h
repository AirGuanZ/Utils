#pragma once

#include <tuple>
#include <utility>

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
            // 下面这行的decltype应该是不必要的，但如果不加，g++ 7.3.0会挂在这
            return std::apply<decltype(&RHS::template Eval<R>)>(&RHS::template Eval<R>,
                              std::tuple_cat(
                                  std::make_tuple<R>(std::forward<R>(range)),
                                  args));
        }
    };

    template<typename R, typename RHS, typename...Args>
    auto operator|(R &&range, AggregateWrapper<RHS, Args...> &&opr)
    {
        return opr.Eval(std::forward<R>(range));
    }
}

} // namespace AGZ
