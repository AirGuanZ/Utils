#pragma once

#include <iterator>
#include <tuple>
#include <utility>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

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
            return std::apply(&RHS::template Eval<R>,
                              std::tuple_cat(
                                std::tuple<R>(std::forward<R>(range)),
                                args));
        }
    };

    template<typename I, typename F>
    struct ReduceRHS
    {
        template<typename R>
        static remove_rcv_t<I> Eval(R &&range, I &&init, F &&func)
        {
            remove_rcv_t<I> ret = init;
            for(auto &&val : range)
                ret = func(ret, val);
            return ret;
        }
    };

    struct CountRHS
    {
        template<typename R>
        static auto Eval(R &&range)
        {
            if constexpr(IsRandomAccessIterator<typename R::Iterator>)
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
        static auto Eval(R &&range, F &&func)
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

    template<typename F>
    struct EachRHS
    {
        template<typename R>
        static auto Eval(R &&range, F &&func)
        {
            for(auto &&v : range)
                func(std::forward<decltype(v)>(v));
            return range;
        }
    };

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

    template<typename F>
    struct AnyRHS
    {
        template<typename R>
        static bool Eval(R &&range, F &&func)
        {
            for(auto &&v : range)
            {
                if(func(std::forward<decltype(v)>(v)))
                    return true;
            }
            return false;
        }
    };
}

template<typename R, typename RHS, typename...Args>
auto operator|(R &&range, RangeAux::AggregateWrapper<RHS, Args...> &&opr)
{
    return opr.Eval(std::forward<R>(range));
}

template<typename I, typename F>
auto Reduce(I &&init, F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::ReduceRHS<I, F>, I, F>(
        std::forward<I>(init), std::forward<F>(func));
}

inline auto Count()
{
    return RangeAux::AggregateWrapper<RangeAux::CountRHS>();
}

template<typename F>
auto CountIf(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::CountIfRHS<F>, F>(
        std::forward<F>(func));
}

template<typename F>
auto Each(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::EachRHS<F>, F>(
        std::forward<F>(func));
}

template<typename F>
bool All(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::AllRHS<F>, F>(
        std::forward<F>(func));
}

template<typename F>
bool Any(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::AnyRHS<F>, F>(
        std::forward<F>(func));
}

AGZ_NS_END(AGZ)
