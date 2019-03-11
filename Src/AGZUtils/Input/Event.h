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
    Manager持有一组Category及其对应的Capturer，负责统一调用capture

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

namespace Impl
{
    template<typename EventParamType>
    class EventHandlerSet;
}

/**
 * @brief 事件处理器接口
 * 
 * 要使 EventHandler 发挥作用，需要将其绑定到特定的 EventCategory 实例上
 * 
 * @tparam EventParamType 事件参数类型。在AGZ::Input中，事件参数类型是事件类型的唯一标识。
 */
template<typename EventParamType>
class EventHandler
{
    friend class Impl::EventHandlerSet<EventParamType>;

    Impl::EventHandlerSet<EventParamType> *handlerSet_;

public:

    EventHandler() noexcept
        : handlerSet_(nullptr)
    {
        
    }

    virtual ~EventHandler();

    /**
     * @brief 处理特定的事件
     */
    virtual void Invoke(const EventParamType &param) = 0;

    /**
     * @brief 该处理器是否已经被绑定到某个 EventCategory
     */
    bool IsAttached() const noexcept
    {
        return handlerSet_ != nullptr;
    }

    /**
     * @brief 若 IsAttached() 为真，解除该绑定关系
     */
    void Detach();
};

/**
 * @cond
 */

namespace Impl
{
    template<typename EventParamType>
    class EventHandlerSet
    {
    protected:

        friend class EventHandler<EventParamType>;

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

        virtual ~EventHandlerSet()
        {
            for(auto handler : handlers_)
                handler->handlerSet_ = nullptr;
        }
    };
}

template<typename EventParamType>
EventHandler<EventParamType>::~EventHandler()
{
    if(handlerSet_)
        handlerSet_->DetachHandlerImpl(this);
}

/**
 * @endcond
 */

/**
 * @brief EventCategory 的公共基类，由内部实现使用
 */
template<typename...EventParamTypes>
class EventCategoryBase : public Impl::EventHandlerSet<EventParamTypes>...
{
protected:

    /**
     * @brief 调用所有绑定到该category实例的事件处理器
     */
    template<typename EventParamType>
    void InvokeAllHandlers(const EventParamType &param)
    {
        Impl::EventHandlerSet<EventParamType>::InvokeAllHandlersImpl(param);
    }

public:

    /**
     * @brief 绑定一个事件处理器
     */
    template<typename EventParamType>
    void AttachHandler(EventHandler<EventParamType> *handler)
    {
        Impl::EventHandlerSet<EventParamType>::AttachHandlerImpl(handler);
    }

    /**
     * @brief 解绑一个事件处理器
     */
    template<typename EventParamType>
    void DetachHandler(EventHandler<EventParamType> *handler)
    {
        Impl::EventHandlerSet<EventParamType>::DetachHandlerImpl(handler);
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

/**
 * @brief EventManager 的公共基类，仅由内部实现使用
 */
template<typename CategoryList, typename CapturerList, std::enable_if_t<
            (std::tuple_size_v<CategoryList> == std::tuple_size_v<CapturerList>), int> = 0>
class EventManagerBase
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

    /**
     * 对每个category，调用其对应的capture捕获事件
     */
    void Capture()
    {
        CallCapture(std::make_index_sequence<std::tuple_size_v<CategoryList>>());
    }

    /**
     * 取得指定类型的category实例
     */
    template<typename CategoryType>
    CategoryType &GetCategoryByType()
    {
        return std::get<CategoryType>(categories_);
    }

    /**
     * 取得指定类型的capturer实例
     */
    template<typename CapturerType>
    CapturerType &GetCapturerByType()
    {
        return std::get<CapturerType>(capturers_);
    }

    /**
     * 取得指定类型的category实例
     */
    template<typename CategoryType>
    const CategoryType &GetCategoryByType() const
    {
        return std::get<CategoryType>(categories_);
    }

    /**
     * 取得指定类型的capturer实例
     */
    template<typename CapturerType>
    const CapturerType &GetCapturerByType() const
    {
        return std::get<CapturerType>(capturers_);
    }
};

} // namespace AGZ::Input
