#pragma once

#include <unordered_set>
#include <tuple>
#include <utility>

#include "../Misc/Common.h"

namespace AGZ::Input
{

/*
    事件管理层次：
        Manager
        Category
        Event

    Event的类型以EventParamType标识，体现为不同类型的EventHandler
    Category持有一些类型的EventHandler的集合，Capturer也是在这一层面定义的
    Manager持有一组Category并记录其对应的Capturer，负责统一调用capture

    // CategoryType是一个可以被看作是一个支持EventParamType的EventCategory
    template<typename CategoryType, typename EventParamType>
    concept EventCategory = 
    requires(CategoryType c, EventParamType e) {
        { c.Invoke(e) };
        requires std::is_same_v<decltype(c.Invoke(e)), void>;
    };

    // 设CategoryType是一个EventCategory，则CapturerType是一个负责其事件捕获的Capturer
    template<typename CapturerType, typename CategoryType>
    concept EventCapturer = EventCategory<CategoryType> &&
    requires(CapturerType cp, CategoryType cc) {
        { cp.Capture(cc) };
    };
*/

template<typename EventParamType>
class EventHandlerSet;

template<typename EventParamType>
class EventHandler
{
    friend class EventHandlerSet<EventParamType>;

    EventHandlerSet<EventParamType> *handlerSet_;

public:

    EventHandler() noexcept
        : handlerSet_(nullptr)
    {
        
    }

    virtual ~EventHandler() = default;

    virtual void Invoke(const EventParamType &param) = 0;

    bool IsAttached() const noexcept
    {
        return handlerSet_ != nullptr;
    }

    void Detach();
};

template<typename EventParamType>
class EventHandlerSet
{
protected:

    std::unordered_set<EventHandler<EventParamType>*> handlers_;

    void InvokeAllHandlersImpl(const EventParamType &param)
    {
        for(auto h : handlers_)
            h->Invoke(param);
    }

    void AttachHandlerImpl(EventHandler<EventParamType> *handler)
    {
        AGZ_ASSERT(handler);
        AGZ_ASSERT(handlers_.find(handler) == handlers_.end());
        AGZ_ASSERT(!handler->IsAttached());
        handlers_.insert(handler);
        handler->handlerSet_ = this;
    }

    void DetachHandlerImpl(EventHandler<EventParamType> *handler)
    {
        AGZ_ASSERT(handler);
        AGZ_ASSERT(handlers_.find(handler) != handlers_.end());
        AGZ_ASSERT(handler->handlerSet_ == this);
        handlers_.erase(handler);
        handler->handlerSet_ = nullptr;
    }

public:

    virtual ~EventHandlerSet() = default;
};

template<typename...EventParamTypes>
class EventCategoryBase : public EventHandlerSet<EventParamTypes>...
{
public:

    template<typename EventParamType>
    void InvokeAllHandlers(const EventParamType &param)
    {
        EventHandlerSet<EventParamType>::InvokeAllHandlersImpl(param);
    }

    template<typename EventParamType>
    void AttachHandler(EventHandler<EventParamType> *handler)
    {
        EventHandlerSet<EventParamType>::AttachHandlerImpl(handler);
    }

    template<typename EventParamType>
    void DetachHandler(EventHandler<EventParamType> *handler)
    {
        EventHandlerSet<EventParamType>::DetachHandlerImpl(handler);
    }
};

template <typename EventParamType>
void EventHandler<EventParamType>::Detach()
{
    if(handlerSet_)
        handlerSet_->DetachHandler(this);
}

template<typename...EventCategoryTypes>
using EventCategoryList = std::tuple<EventCategoryTypes...>;

template<typename...EventCapturerTypes>
using EventCapturerList = std::tuple<EventCapturerTypes...>;

template<typename CategoryList, typename CapturerList, std::enable_if_t<
            (std::tuple_size_v<CategoryList> == std::tuple_size_v<CapturerList>), int> = 0>
class EventManager
{
    CategoryList categories_;
    CapturerList capturers_;

    template<size_t N>
    void CallCapture()
    {
        std::get<N>(capturers_).Capture(std::get<N>(categories_));
    }

    template<size_t N, size_t N1, size_t...Ns>
    void CallCapture()
    {
        CallCapture<N>();
        CallCapture<N1, Ns...>();
    }

    template<size_t...Ns>
    void CallCapture(std::index_sequence<Ns...>)
    {
        CallCapture<Ns...>();
    }

public:

    void Capture()
    {
        CallCapture(std::make_index_sequence<std::tuple_size_v<CategoryList>>());
    }

    template<typename CategoryType>
    CategoryType &GetCategoryByType()
    {
        return std::get<CategoryType>(categories_);
    }

    template<typename CapturerType>
    CapturerType &GetCapturerByType()
    {
        return std::get<CapturerType>(capturers_);
    }

    template<typename CategoryType>
    const CategoryType &GetCategoryByType() const
    {
        return std::get<CategoryType>(categories_);
    }

    template<typename CapturerType>
    const CapturerType &GetCapturerByType() const
    {
        return std::get<CapturerType>(capturers_);
    }
};

} // namespace AGZ::Input
