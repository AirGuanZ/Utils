#pragma once

#include <functional>

#include "Common.h"

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

/**
 * @brief 定义一个自动命名的scope guard，离开定义作用域时自动调用指定的操作
 * 
 * 由于命名是根据行号自动进行的，很难对其调用dismiss操作。如有此需要，可显式使用 AGZ::ScopeGuard
 */
#define AGZ_SCOPE_GUARD(X)  AGZ_SCOPE_GUARD_IMPL0(X, __LINE__)
#define AGZ_SCOPE_GUARD_IMPL0(X, LINE)  AGZ_SCOPE_GUARD_IMPL1(X, LINE)
#define AGZ_SCOPE_GUARD_IMPL1(X, LINE) ::AGZ::ScopeGuard _autoScopeGuard##LINE([&] X)
