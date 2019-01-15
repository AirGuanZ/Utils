#pragma once

// include glfw before this header

#ifndef _glfw3_h_
#   error "Include glfw before this header"
#endif

#ifdef AGZ_USE_GLFW

#include <unordered_map>
#include <vector>

#include "../Misc/TypeOpr.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Window.h"

namespace AGZ::Input
{

class GLFWKeyboardCapturer;
class GLFWMouseCapturer;
class GLFWWindowCapturer;

/**
 * @cond
 */

namespace Impl
{
    inline std::unordered_map<GLFWwindow*, GLFWKeyboardCapturer*> GLFWWindow2KeyboardCapturer;
    inline std::unordered_map<GLFWwindow*, GLFWMouseCapturer*> GLFWWindow2MouseCapturer;
    inline std::unordered_map<GLFWwindow*, GLFWWindowCapturer*> GLFWWindow2WindowCapturer;
}

/**
 * @endcond
 */

inline void GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

inline void GLFWCursorMoveCallback      (GLFWwindow *window, double xpos, double ypos);
inline void GLFWMouseButtonCallback     (GLFWwindow *window, int button, int action, int mods);
inline void GLFWCursorEnterLeaveCallback(GLFWwindow *window, int entered);
inline void GLFWWheelScrollCallback     (GLFWwindow *window, double xoffset, double yoffset);

inline void GLFWWindowCloseCallback          (GLFWwindow *window);
inline void GLFWWindowSizeCallback           (GLFWwindow *window, int width, int height);
inline void GLFWWindowFramebufferSizeCallback(GLFWwindow *window, int width, int height);

/**
 * @brief 适用于glfw3的键盘事件捕获器
 */
class GLFWKeyboardCapturer
{
    GLFWwindow *window_ = nullptr;

    friend void GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    struct EventRecord
    {
        int key;
        int action;
    };

    bool isFirstCapture_;
    std::vector<EventRecord> eventRecords_;

    void AddEventRecord(int key, int action)
    {
        eventRecords_.push_back({ key, action });
    }

public:

    GLFWKeyboardCapturer()
        : isFirstCapture_(true)
    {
        
    }

    ~GLFWKeyboardCapturer()
    {
        auto it = Impl::GLFWWindow2KeyboardCapturer.find(window_);
        AGZ_ASSERT(it != Impl::GLFWWindow2KeyboardCapturer.end());
        AGZ_ASSERT(it->second == this);
        Impl::GLFWWindow2KeyboardCapturer.erase(it);
    }

    /**
     * @brief 初始化捕获器，向glfw注册键盘事件回调
     * 
     * 在第一次使用前必须调用，且在该捕获器生命周期中仅能调用一次
     * 
     * @param window glfw窗口句柄
     */
    void Initialize(GLFWwindow *window)
    {
        AGZ_ASSERT(window && !window_);
        auto it = Impl::GLFWWindow2KeyboardCapturer.find(window);
        if(it != Impl::GLFWWindow2KeyboardCapturer.end())
            it->second = this;
        else
        {
            glfwSetKeyCallback(window, GLFWKeyCallback);
            Impl::GLFWWindow2KeyboardCapturer[window] = this;
        }
        window_ = window;
    }

    /**
     * @brief 捕获由glfw给出的键盘事件
     */
    void Capture(Keyboard &keyboard)
    {
        if(isFirstCapture_)
        {
            for(int k = 0; k <= KEY_MAX; ++k)
            {
                if(glfwGetKey(window_, k) == GLFW_PRESS)
                    keyboard._setKeyPressed(k, true);
            }

            isFirstCapture_ = false;
        }

        for(auto &er : eventRecords_)
        {
            if(er.action == GLFW_PRESS)
                keyboard.Invoke(KeyDown{ Key(er.key) });
            else if(er.action == GLFW_RELEASE)
                keyboard.Invoke(KeyUp{ Key(er.key) });
        }
        eventRecords_.clear();
    }
};

/**
 * @brief 适用于glfw3的鼠标事件捕获器
 */
class GLFWMouseCapturer
{
    GLFWwindow *window_ = nullptr;

    friend void GLFWCursorMoveCallback      (GLFWwindow *window, double xpos, double ypos);
    friend void GLFWMouseButtonCallback     (GLFWwindow *window, int button, int action, int mods);
    friend void GLFWCursorEnterLeaveCallback(GLFWwindow *window, int entered);
    friend void GLFWWheelScrollCallback     (GLFWwindow *window, double xoffset, double yoffset);

    struct MoveEventRecord
    {
        double xpos;
        double ypos;
    };

    struct ButtonEventRecord
    {
        int button;
        int action;
    };

    struct EnterLeaveRecord
    {
        bool entered;
    };

    struct ScrollEventRecord
    {
        double offset;
    };

    using EventRecord = TypeOpr::Variant<
        MoveEventRecord, ButtonEventRecord, EnterLeaveRecord, ScrollEventRecord>;

    bool isFirstCapture_;
    std::vector<EventRecord> eventRecords_;

public:

    GLFWMouseCapturer()
        : isFirstCapture_(true)
    {
        
    }

    ~GLFWMouseCapturer()
    {
        auto it = Impl::GLFWWindow2MouseCapturer.find(window_);
        AGZ_ASSERT(it != Impl::GLFWWindow2MouseCapturer.end());
        AGZ_ASSERT(it->second == this);
        Impl::GLFWWindow2MouseCapturer.erase(it);
    }

    /**
     * @brief 初始化捕获器，向glfw注册鼠标事件回调
     *
     * 在第一次使用前必须调用，且在该捕获器生命周期中仅能调用一次
     *
     * @param window glfw窗口句柄
     */
    void Initialize(GLFWwindow *window)
    {
        AGZ_ASSERT(window && !window_);
        auto it = Impl::GLFWWindow2MouseCapturer.find(window);
        if(it != Impl::GLFWWindow2MouseCapturer.end())
            it->second = this;
        else
        {
            glfwSetCursorPosCallback(window, GLFWCursorMoveCallback);
            glfwSetMouseButtonCallback(window, GLFWMouseButtonCallback);
            glfwSetCursorEnterCallback(window, GLFWCursorEnterLeaveCallback);
            glfwSetScrollCallback(window, GLFWWheelScrollCallback);
            Impl::GLFWWindow2MouseCapturer[window] = this;
        }
        window_ = window;
    }

    /**
     * @brief 捕获由glfw给出的鼠标事件
     */
    void Capture(Mouse &mouse)
    {
        if(isFirstCapture_)
        {
            double x, y;
            glfwGetCursorPos(window_, &x, &y);
            mouse._setCursorPosition(x, y);

            if(glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                mouse._setButtonPressed(MOUSE_LEFT, true);
            if(glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
                mouse._setButtonPressed(MOUSE_MIDDLE, true);
            if(glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
                mouse._setButtonPressed(MOUSE_RIGHT, true);

            isFirstCapture_ = false;
        }

        for(auto &er : eventRecords_)
        {
            TypeOpr::MatchVar(er,
            [&](const MoveEventRecord &e)
            {
                CursorMove param =
                {
                    e.xpos, e.ypos,
                    e.xpos - mouse.GetCursorPositionX(),
                    e.ypos - mouse.GetCursorPositionY()
                };
                mouse.Invoke(param);
            },
            [&](const ButtonEventRecord &e)
            {
                MouseButton button;
                if(e.button == GLFW_MOUSE_BUTTON_LEFT)
                    button = MOUSE_LEFT;
                else if(e.button == GLFW_MOUSE_BUTTON_MIDDLE)
                    button = MOUSE_MIDDLE;
                else if(e.button == GLFW_MOUSE_BUTTON_RIGHT)
                    button = MOUSE_RIGHT;
                else
                    return;

                if(e.action == GLFW_PRESS)
                    mouse.Invoke(MouseButtonDown{ button });
                else if(e.action == GLFW_RELEASE)
                    mouse.Invoke(MouseButtonUp{ button });
            },
            [&](const EnterLeaveRecord &e)
            {
                if(e.entered)
                    mouse.Invoke(CursorEnter{});
                else
                    mouse.Invoke(CursorLeave{});
            },
            [&](const ScrollEventRecord &e)
            {
                mouse.Invoke(WheelScroll{ e.offset });
            });
        }
        eventRecords_.clear();
    }
};

/**
 * @brief 适用于glfw3的窗口事件捕获器
 */
class GLFWWindowCapturer
{
    struct ClosedEventRecord { };
    struct SizeEventRecord { int w, h; };
    struct FramebufferSizeEventRecord { int w, h; };

    using EventRecord = TypeOpr::Variant<
        ClosedEventRecord, SizeEventRecord, FramebufferSizeEventRecord>;

    friend void GLFWWindowCloseCallback          (GLFWwindow *window);
    friend void GLFWWindowSizeCallback           (GLFWwindow *window, int width, int height);
    friend void GLFWWindowFramebufferSizeCallback(GLFWwindow *window, int width, int height);

    GLFWwindow *window_;

    std::vector<EventRecord> eventRecords_;

public:

    ~GLFWWindowCapturer()
    {
        auto it = Impl::GLFWWindow2WindowCapturer.find(window_);
        AGZ_ASSERT(it != Impl::GLFWWindow2WindowCapturer.end());
        AGZ_ASSERT(it->second == this);
        Impl::GLFWWindow2WindowCapturer.erase(it);
    }

    /**
     * @brief 初始化捕获器，向glfw注册窗口事件回调
     *
     * 在第一次使用前必须调用，且在该捕获器生命周期中仅能调用一次
     *
     * @param window glfw窗口句柄
     */
    void Initialize(GLFWwindow *window)
    {
        AGZ_ASSERT(window && !window_);
        auto it = Impl::GLFWWindow2WindowCapturer.find(window);
        if(it != Impl::GLFWWindow2WindowCapturer.end())
            it->second = this;
        else
        {
            glfwSetWindowCloseCallback(window, GLFWWindowCloseCallback);
            glfwSetWindowSizeCallback(window, GLFWWindowSizeCallback);
            glfwSetFramebufferSizeCallback(window, GLFWWindowFramebufferSizeCallback);
            Impl::GLFWWindow2WindowCapturer[window] = this;
        }
        window_ = window;
    }

    /**
     * @brief 捕获由glfw给出的窗口事件
     */
    void Capture(Window &window)
    {
        for(auto &er : eventRecords_)
        {
            TypeOpr::MatchVar(er,
            [&](const SizeEventRecord &e)
            {
                window.Invoke(WindowSize{ e.w, e.h });
            },
            [&](const ClosedEventRecord &)
            {
                window.Invoke(WindowClose{});
            },
            [&](const FramebufferSizeEventRecord &e)
            {
                window.Invoke(FramebufferSize{ e.w, e.h });
            });
        }
        eventRecords_.clear();
    }
};

/**
 * @cond
 */

inline void GLFWKeyCallback(GLFWwindow *window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods)
{
    auto it = Impl::GLFWWindow2KeyboardCapturer.find(window);
    if(it == Impl::GLFWWindow2KeyboardCapturer.end())
        return;
    auto cpr = it->second;
    cpr->AddEventRecord(key, action);
}

inline void GLFWCursorMoveCallback(GLFWwindow *window, double xpos, double ypos)
{
    auto it = Impl::GLFWWindow2MouseCapturer.find(window);
    if(it == Impl::GLFWWindow2MouseCapturer.end())
        return;
    it->second->eventRecords_.emplace_back(GLFWMouseCapturer::MoveEventRecord{ xpos, ypos });
}

inline void GLFWMouseButtonCallback(GLFWwindow *window, int button, int action, [[maybe_unused]] int mods)
{
    auto it = Impl::GLFWWindow2MouseCapturer.find(window);
    if(it == Impl::GLFWWindow2MouseCapturer.end())
        return;
    it->second->eventRecords_.emplace_back(GLFWMouseCapturer::ButtonEventRecord{ button, action });
}

inline void GLFWCursorEnterLeaveCallback(GLFWwindow *window, int entered)
{
    auto it = Impl::GLFWWindow2MouseCapturer.find(window);
    if(it == Impl::GLFWWindow2MouseCapturer.end())
        return;
    it->second->eventRecords_.emplace_back(GLFWMouseCapturer::EnterLeaveRecord{ entered != 0 });
}

inline void GLFWWheelScrollCallback(GLFWwindow *window, [[maybe_unused]] double xoffset, double yoffset)
{
    auto it = Impl::GLFWWindow2MouseCapturer.find(window);
    if(it == Impl::GLFWWindow2MouseCapturer.end())
        return;
    it->second->eventRecords_.emplace_back(GLFWMouseCapturer::ScrollEventRecord{ yoffset });
}

inline void GLFWWindowSizeCallback(GLFWwindow *window, int width, int height)
{
    auto it = Impl::GLFWWindow2WindowCapturer.find(window);
    if(it == Impl::GLFWWindow2WindowCapturer.end())
        return;
    it->second->eventRecords_.emplace_back(GLFWWindowCapturer::SizeEventRecord{ width, height });
}

inline void GLFWWindowCloseCallback(GLFWwindow *window)
{
    auto it = Impl::GLFWWindow2WindowCapturer.find(window);
    if(it == Impl::GLFWWindow2WindowCapturer.end())
        return;
    it->second->eventRecords_.emplace_back(GLFWWindowCapturer::ClosedEventRecord{});
}

inline void GLFWWindowFramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    auto it = Impl::GLFWWindow2WindowCapturer.find(window);
    if(it == Impl::GLFWWindow2WindowCapturer.end())
        return;
    it->second->eventRecords_.emplace_back(GLFWWindowCapturer::FramebufferSizeEventRecord{ width, height });
}

/**
 * @endcond
 */

} // namespace AGZ::Input

#endif // #ifdef AGZ_USE_GLFW
