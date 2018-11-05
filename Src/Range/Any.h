#pragma once

#pragma once

#include "../Misc/Common.h"
#include "Aggregate.h"

namespace AGZ {

namespace RangeAux
{
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

/**
 * @brief 判定一个range对象中是否存在满足某个谓词的元素
 * 
 * @note 该聚合函数可能导致range中所有元素被求值
 * 
 * @param func 谓词函数，接受range中的元素，返回bool类型
 */
template<typename F>
bool Any(F &&func)
{
    return RangeAux::AggregateWrapper<RangeAux::AnyRHS<
                remove_rcv_t<F>>, remove_rcv_t<F>>(
                    std::forward<F>(func));
}

} // namespace AGZ
