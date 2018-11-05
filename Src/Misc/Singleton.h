#pragma once

#include <memory>

#include "Common.h"

namespace AGZ {

/**
 * @brief 单件模式模板
 * 
 * 使用时令单件类T继承Singleton<T>即可
 */
template<typename T>
class Singleton
{
public:

    static T &GetInstance()
    {
        // thread-safe after C++11
        static T ret;
        return ret;
    }

    static T *GetInstancePtr()
    {
        return &GetInstance();
    }
};

} // namespace AGZ
