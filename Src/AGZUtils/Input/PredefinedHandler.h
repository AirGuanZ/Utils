#pragma once

#include <functional>
#include <type_traits>

#include "Event.h"

namespace AGZ::Input
{
    
template<typename EventParamType>
class FunctionalEventHandler : public EventHandler<EventParamType>
{
public:

    using Func_t = std::function<void(const EventParamType&)>;

    static constexpr struct NOP_t { constexpr NOP_t() { } operator Func_t() { return Func_t(); } } NOP = NOP_t();

    template<typename T>
    explicit FunctionalEventHandler(T &&f)
        : func_(std::forward<T>(f))
    {
        
    }

    void Invoke(const EventParamType &param) override
    {
        if(func_)
            func_(param);
    }

private:

    Func_t func_;
};

template<typename EventParamType, typename ClassType, std::enable_if_t<std::is_class_v<ClassType>, int> = 0>
class MemberFunctionEventHandler : public EventHandler<EventParamType>
{
public:

    using MemFuncPtr_t = void(ClassType::*)(const EventParamType&);

    MemberFunctionEventHandler(ClassType *c, MemFuncPtr_t m) noexcept
        : class_(c), memFunc_(m)
    {
        
    }

    void Invoke(const EventParamType &param) override
    {
        class_->memFunc_(param);
    }

private:

    ClassType *class_;
    MemFuncPtr_t memFunc_;
};

} // namespace AGZ::Input
