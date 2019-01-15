#pragma once

#include "Event.h"
#include "PredefinedHandler.h"

namespace AGZ::Input
{

/**
 * @brief 窗口大小改变事件
 */
struct WindowSize
{
    int w, h;
};

/**
 * @brief 窗口关闭事件
 * 注意这并不是说窗口要被销毁了，只是表示用户触发了“关闭窗口”这一动作，比如按了alt+f4
 */
struct WindowClose { };

/**
 * @brief Framebuffer大小改变事件
 * 仅在使用dx/ogl/vk等graphics api时有意义
 */
struct FramebufferSize
{
    int w, h;
};

class Window :
    public EventCategoryBase<WindowSize, WindowClose, FramebufferSize>
{
public:

    void Invoke(const WindowSize &param)
    {
        InvokeAllHandlers(param);
    }

    void Invoke(const WindowClose &param)
    {
        InvokeAllHandlers(param);
    }

    void Invoke(const FramebufferSize &param)
    {
        InvokeAllHandlers(param);
    }
};

PREDEFINED_HANDLER_FOR_SPECIFIC_EVENT(WindowSize);
PREDEFINED_HANDLER_FOR_SPECIFIC_EVENT(WindowClose);
PREDEFINED_HANDLER_FOR_SPECIFIC_EVENT(FramebufferSize);

template<typename CapturerType>
class WindowManager : public EventManagerBase<EventCategoryList<Window>,
                                              EventCapturerList<CapturerType>>
{
public:

    /**
     * @brief 取得窗口实例
     */
    Window &GetWindow() noexcept
    {
        return this->template GetCategoryByType<Window>();
    }

    /**
     * @brief 取得捕获器实例
     */
    CapturerType &GetCapturer() noexcept
    {
        return this->template GetCapturerByType<CapturerType>();
    }

    /**
     * @brief 取得窗口实例
     */
    const Window &GetWindow() const noexcept
    {
        return this->template GetCategoryByType<Window>();
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
