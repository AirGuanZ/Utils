#pragma once

#include "Common.h"

namespace AGZ {

/**
 * @brief 不可复制类模板
 * 
 * 凡继承此类者均无默认拷贝构造和赋值函数
 */
class Uncopiable
{
public:

    Uncopiable()                             = default;
    Uncopiable(const Uncopiable&)            = delete;
    Uncopiable &operator=(const Uncopiable&) = delete;
};

} // namespace AGZ
