#pragma once

#include <memory>

#include "Common.h"

namespace AGZ {

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
