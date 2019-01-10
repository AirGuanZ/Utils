#pragma once

#include <list>
#include <set>
#include <unordered_set>
#include <vector>

#include "../Misc/Common.h"
#include "Aggregate.h"

namespace AGZ {

namespace RangeAux
{
    template<typename C>
    struct CollectAsPushBackableRHS
    {
        template<typename R>
        static C Eval(R &&range)
        {
             C ret;
             for(auto &&val : range)
                ret.push_back(std::forward<decltype(val)>(val));
            return ret;
        }
    };

    template<typename C>
    struct CollectAsInsertableRHS
    {
        template<typename R>
        static C Eval(R &&range)
        {
             C ret;
             for(auto &&v : range)
                ret.insert(std::forward<decltype(v)>(v));
            return ret;
        }
    };

    template<typename T> struct CollectorSelector { };

    template<typename T>
    struct CollectorSelector<std::vector<T>>
    {
        using RHS = CollectAsPushBackableRHS<std::vector<T>>;
    };

    template<typename T>
    struct CollectorSelector<std::list<T>>
    {
        using RHS = CollectAsPushBackableRHS<std::list<T>>;
    };

    template<typename T>
    struct CollectorSelector<std::set<T>>
    {
        using RHS = CollectAsInsertableRHS<std::set<T>>;
    };

    template<typename T>
    struct CollectorSelector<std::unordered_set<T>>
    {
        using RHS = CollectAsInsertableRHS<std::unordered_set<T>>;
    };
}

/**
 * @brief 将某个range中的元素按遍历次序收集到某个容器中
 * 
 * 目前支持的容器包括：
 * - std::vector
 * - std::list
 * - std::set
 * - ste::unordered_set
 * 
 * 理论上可以通过进一步的SFINAE把该函数推广到所有拥有insert/push/push_back的容器上，
 * 但是我暂时用不着
 */
template<typename T>
auto Collect()
{
    return RangeAux::AggregateWrapper<
            typename RangeAux::CollectorSelector<T>::RHS>();
}

} // namespace AGZ
