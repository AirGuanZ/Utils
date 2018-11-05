#pragma once

#include "../Misc/Common.h"
#include "Aggregate.h"

namespace AGZ {

namespace RangeAux
{
    template<typename F>
    struct EachRHS
    {
        template<typename R>
        static auto &&Eval(R &&range, F &&func)
        {
            for(auto &&v : range)
                func(std::forward<decltype(v)>(v));
            return std::forward<R>(range);
        }
    };

    template<typename F>
    struct EachIndexRHS
    {
        template<typename R>
        static auto &&Eval(R &&range, F &&func)
        {
            size_t i = 0;
            for(auto &&v : range)
                func(std::forward<decltype(v)>(v), i++);
            return std::forward<R>(range);
        }
    };
}

/**
 * @brief 以range中的每个元素为参数，分别调用给定的函数
 * 
 * @return 未求值的原range
 * 
 * @note 该函数会导致range中的每一个元素被求一次值
 */
template<typename F>
auto Each(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::EachRHS<
                remove_rcv_t<F>>, remove_rcv_t<F>>(
                    std::forward<F>(func));
}

/**
 * @brief 对range中的每一个元素，用它和它的下标调用给定的函数
 * 
 * @return 未求值的原range
 * 
 * @note 该函数会导致range中的每一个元素被求一次值
 */
template<typename F>
auto EachIndex(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::EachIndexRHS<
                remove_rcv_t<F>>, remove_rcv_t<F>>(
                    std::forward<F>(func));
}

} // namespace AGZ
