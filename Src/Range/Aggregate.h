#pragma once

#include <iterator>
#include <tuple>
#include <utility>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<template<typename...Args> class RHS>
    struct AggregateWrapper
    {
        std::tuple<Args...> args;

        AggregateWrapper(Args&&...args)
            : args(std::forward<Args>(args)...)
        {

        }

        template<typename R>
        auto Eval(R &&range)
        {
            return std::apply(&RHS::Eval<R>, std::tuple_cat(
                        std::tuple<R>(std::forward<R>(range)),
                        args));
        }
    };

    struct ReduceRHS
    {
        template<typename R, typename I, typename F>
        auto Eval(R &&range, I &&init, F &&func)
        {
            auto ret = init;
            for(auto &val : range)
                ret = func(ret, init);
            return ret;
        }
    };

    struct CountRHS
    {
        template<typename R>
        auto Eval(const R &range)
        {
            if constexpr(std::is_base_of_v<
                                std::random_access_iterator_tag,
                                typename R::Iterator::iterator_category>)
            {
                return std::end(range) - std::begin(range);
            }
            else
            {
                typename R::Iterator::difference_type ret = 0;
                auto it = std::begin(range), end = std::end(range);
                while(it++ != end)
                    ++ret;
                return ret;
            }
        }
    };

    template<typename F>
    struct CountIfRHS
    {
        template<typename R>
        auto Eval(const R &range, F &&func)
        {
            typename R::Iterator::difference_type ret = 0;
            auto it = std::begin(range), end = std::end(range);
            while(it != end)
            {
                if(func(*it++))
                    ++ret;
            }
            return ret;
        }
    };
}

template<typename R, typename RHS>
auto operator|(R &&range, const RangeAux::AggregateWrapper<RHS> &opr)
{
    return opr.Eval(std::forward<R>(range));
}

template<typename I, typename F>
auto Reduce(I &init, F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::ReduceRHS<I, F>>(
        std::forward<I>(init), std::forward<F>(func));
}

auto Count()
{
    return RangeAux::AggregateWrapper<RangeAux::CountRHS>();
}

template<typename F>
auto CountIf(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::CountIfRHS<F>>(
        std::forward<F>(func));
}

AGZ_NS_END(AGZ)
