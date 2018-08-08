#pragma once

#include "../Misc/Common.h"
#include "Aggregate.h"
#include "Iterator.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    struct CountRHS
    {
        template<typename R>
        static auto Eval(R &&range)
        {
            if constexpr(IsRandomAccessIterator<GetIteratorType<R>>)
            {
                return std::end(range) - std::begin(range);
            }
            else
            {
                typename std::iterator_traits<
                    GetIteratorType<R>>::difference_type ret = 0;
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
            typename std::iterator_traits<GetIteratorType<R>>
                ::difference_type ret = 0;
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

AGZ_NS_END(AGZ)
