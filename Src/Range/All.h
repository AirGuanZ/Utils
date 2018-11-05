#pragma once

#include "../Misc/Common.h"
#include "Aggregate.h"

namespace AGZ {

namespace RangeAux
{
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
}

/**
 * @brief 判定一个range对象中是否所有元素都满足某个谓词
 * 
 * @note 该聚合函数会导致range中所有元素被求值
 * 
 * @param func 谓词函数，接受range中的元素，返回bool类型
 */
template<typename F>
bool All(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::AllRHS<
                remove_rcv_t<F>>, remove_rcv_t<F>>(
                    std::forward<F>(func));
}

} // namespace AGZ
