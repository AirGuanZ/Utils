#pragma once

#include "../Misc/Common.h"
#include "Aggregate.h"
#include "Iterator.h"

namespace AGZ {

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

/**
 * @brief 计算一个range中元素的数量
 * 
 * @note 该聚合函数会导致range中所有的元素被求值
 */
inline auto Count()
{
    return RangeAux::AggregateWrapper<RangeAux::CountRHS>();
}

/**
 * @brief 计算一个range中满足某谓词的元素数量
 * 
 * @note 该聚合函数可能导致range中所有元素被求值
 */
template<typename F>
auto CountIf(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::CountIfRHS<F>, F>(
                    std::forward<F>(func));
}

} // namespace AGZ
