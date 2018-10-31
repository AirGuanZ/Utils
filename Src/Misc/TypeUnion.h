#pragma once

#include <type_traits>
#include <variant>

#include "Common.h"

AGZ_NS_BEG(AGZ)

template<typename...Ts>
using Variant = std::variant<Ts...>;

template<typename R = void, typename E, typename...Vs>
R MatchVar(E &&e, Vs...vs)
{
    struct overloaded : Vs...
    {
        explicit overloaded(Vs...vss)
            : Vs(vss)...
        {

        }
    };

    return std::visit(
        overloaded(vs...),
        std::forward<E>(e));
}

AGZ_NS_END(AGZ)
