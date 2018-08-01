#pragma once

#include <tuple>
#include <utility>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<typename ImplTrait, typename...Args>
    struct TransformWrapper
    {
        std::tuple<Args...> args;

        explicit TransformWrapper(Args&&..._args)
            : args(std::forward<Args>(_args)...)
        {

        }

        template<typename R>
        auto Eval(R &&range)
        {
            return std::make_from_tuple<
                        typename ImplTrait::template Impl<R>>(
                            std::tuple_cat(
                                std::tuple<remove_rcv_t<R>>(
                                    std::forward<R>(range)),
                                args));
        }
    };
}

// TODO: 把所有range的transformer都改写成TransformWrapper的形式

template<typename R, typename Impl, typename...Args>
auto operator|(R &&range, RangeAux::TransformWrapper<Impl, Args...> &&opr)
{
    return opr.Eval(std::forward<R>(range));
}

AGZ_NS_END(AGZ)
