#pragma once

#include <tuple>
#include <utility>

#include "../Misc/Common.h"

namespace AGZ {

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
                        typename ImplTrait::template Impl<remove_rcv_t<R>>>(
                            std::tuple_cat(
                                std::tuple<remove_rcv_t<R>>(
                                    std::forward<R>(range)),
                                args));
        }
    };

    template<typename R, typename Impl, typename...Args>
    auto operator|(R &&range, TransformWrapper<Impl, Args...> &&opr)
    {
        return opr.Eval(std::forward<R>(range));
    }
}

} // namespace AGZ
