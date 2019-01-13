#pragma once

#include "Event.h"
#include "PredefinedHandler.h"

namespace AGZ::Input
{

/**
 * @brief 鼠标按钮
 */
using MouseButton = std::int32_t;

constexpr MouseButton MOUSE_LEFT   = 0;
constexpr MouseButton MOUSE_MIDDLE = 1;
constexpr MouseButton MOUSE_RIGHT  = 2;

/**
 * @brief 鼠标按钮按下事件
 */
struct MouseButtonDown
{
    MouseButton button;
};

/**
 * @brief 鼠标按钮松开事件
 */
struct MouseButtonUp
{
    MouseButton button;
};

/**
 * @brief 光标进入窗口区域事件
 */
struct CursorEnter { };

/**
 * @brief 光标离开窗口区域事件
 */
struct CursorLeave { };

/**
 * @brief 光标移动事件
 */
struct CursorMove
{
    double absX, absY;
    double relX, relY;
};

/**
 * @brief 滑轮滚动事件
 */
struct WheelScroll { double offset; };

/**
 * @brief 鼠标事件category
 */
class Mouse :
    public EventCategoryBase<MouseButtonDown, MouseButtonUp,
                             CursorEnter, CursorLeave, CursorMove,
                             WheelScroll>
{
    bool isButtonPressed_[3];
    double absX_, absY_;

public:

    Mouse() noexcept
    {
        std::memset(isButtonPressed_, 0, sizeof(isButtonPressed_));
        absX_ = 0; absY_ = 0;
    }

    void Invoke(const MouseButtonDown &param)
    {
        isButtonPressed_[param.button] = true;
        InvokeAllHandlers(param);
    }

    void Invoke(const MouseButtonUp &param)
    {
        isButtonPressed_[param.button] = false;
        InvokeAllHandlers(param);
    }

    void Invoke(const CursorEnter &param)
    {
        InvokeAllHandlers(param);
    }

    void Invoke(const CursorLeave &param)
    {
        InvokeAllHandlers(param);
    }

    void Invoke(const CursorMove &param)
    {
        absX_ = param.absX;
        absY_ = param.absY;
        InvokeAllHandlers(param);
    }

    void Invoke(const WheelScroll &param)
    {
        InvokeAllHandlers(param);
    }

    bool IsMouseButtonPressed(MouseButton button) const noexcept
    {
        AGZ_ASSERT(0 <= button && button < 3);
        return isButtonPressed_[button];
    }

    double GetCursorPositionX() const noexcept
    {
        return absX_;
    }

    double GetCursorPositionY() const noexcept
    {
        return absY_;
    }
};

PREDEFINED_HANDLER_FOR_SPECIFIC_EVENT(MouseButtonDown);
PREDEFINED_HANDLER_FOR_SPECIFIC_EVENT(MouseButtonUp);
PREDEFINED_HANDLER_FOR_SPECIFIC_EVENT(CursorEnter);
PREDEFINED_HANDLER_FOR_SPECIFIC_EVENT(CursorLeave);
PREDEFINED_HANDLER_FOR_SPECIFIC_EVENT(CursorMove);
PREDEFINED_HANDLER_FOR_SPECIFIC_EVENT(WheelScroll);

/**
 * @brief 鼠标管理器，负责打包鼠标category和指定的事件捕获器
 * @tparam CapturerType 捕获器类型
 */
template<typename CapturerType>
class MouseManager : public EventManagerBase<EventCategoryList<Mouse>, EventCapturerList<CapturerType>>
{
public:

    /**
     * @brief 取得鼠标实例
     */
    Mouse &GetMouse() noexcept
    {
        return this->template GetCategoryByType<Mouse>();
    }

    /**
     * @brief 取得捕获器实例
     */
    CapturerType &GetCapturer() noexcept
    {
        return this->template GetCapturerByType<CapturerType>();
    }

    /**
     * @brief 取得鼠标实例
     */
    const Mouse &GetMouse() const noexcept
    {
        return this->template GetCategoryByType<Mouse>();
    }

    /**
     * @brief 取得捕获器实例
     */
    const CapturerType &GetCapturer() const noexcept
    {
        return this->template GetCapturerByType<CapturerType>();
    }
};

} // namespace AGZ::Input
