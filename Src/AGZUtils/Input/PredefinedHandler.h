#pragma once

#include <functional>
#include <type_traits>

#include "Event.h"

namespace AGZ::Input
{
    
/**
 * @brief 将一个可调用对象封装为指定类型的事件处理器
 */
template<typename EventParamType>
class FunctionalEventHandler : public EventHandler<EventParamType>
{
public:

    using Func_t = std::function<void(const EventParamType&)>;

    static constexpr struct NOP_t { operator Func_t() { return Func_t(); } } NOP = NOP_t();

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

/**
 * @brief 将一个类实例及其成员函数指针包装成指定类型的事件处理器
 */
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

#define PREDEFINED_HANDLER_FOR_SPECIFIC_EVENT(EventType) \
    using EventType##Handler = FunctionalEventHandler<EventType>; \
    template<typename C> using MemFn##EventType##Handler = MemberFunctionEventHandler<EventType, C>

} // namespace AGZ::Input
