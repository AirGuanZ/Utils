#pragma once

#include <functional>

#include "Uncopiable.h"

namespace AGZ
{
    
class ScopeGuard : public Uncopiable
{
    std::function<void()> func_;

public:

    template<typename T>
    explicit ScopeGuard(T &&func)
        : func_(std::forward<T>(func))
    {
        
    }

    ~ScopeGuard()
    {
        if(func_)
            func_();
    }

    void Dismiss()
    {
        func_ = std::function<void()>();
    }
};

} // namespace AGZ
