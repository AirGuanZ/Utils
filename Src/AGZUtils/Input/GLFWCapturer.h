#pragma once

// include glfw before this header

#ifdef AGZ_USE_GLFW

#include <unordered_map>
#include <vector>

#include "Keyboard.h"
#include "Mouse.h"

namespace AGZ::Input
{

class GLFWKeyboardCapturer;
class GLFWMouseCapturer;

/**
 * @cond
 */

namespace Impl
{
    inline std::unordered_map<GLFWwindow*, GLFWKeyboardCapturer*> GLFWWindow2KeyboardCapturer;
    inline std::unordered_map<GLFWwindow*, GLFWMouseCapturer*> GLFWWindow2MouseCapturer;
}

/**
 * @endcond
 */

inline void GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

inline void GLFWCursorMoveCallback(GLFWwindow *window, double xpos, double ypos);
inline void GLFWMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
inline void GLFWCursorEnterLeaveCallback(GLFWwindow *window, int entered);
inline void GLFWWheelScrollCallback(GLFWwindow *window, double xoffset, double yoffset);

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

    std::vector<EventRecord> eventRecords_;

    void AddEventRecord(int key, int action)
    {
        eventRecords_.push_back({ key, action });
    }

public:

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

    friend void GLFWCursorMoveCallback(GLFWwindow *window, double xpos, double ypos);
    friend void GLFWMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    friend void GLFWCursorEnterLeaveCallback(GLFWwindow *window, int entered);
    friend void GLFWWheelScrollCallback(GLFWwindow *window, double xoffset, double yoffset);

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

    enum class Type
    {
        Move, Button, EnterLeave, Scroll
    };

    struct EventRecord
    {
        Type type;
        union
        {
            MoveEventRecord move;
            ButtonEventRecord button;
            EnterLeaveRecord enterLeave;
            ScrollEventRecord scroll;
        };
    };

    std::vector<EventRecord> eventRecords_;

public:

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
        for(auto &er : eventRecords_)
        {
            if(er.type == Type::Move)
            {
                CursorMove param =
                {
                    er.move.xpos, er.move.ypos,
                    er.move.xpos - mouse.GetCursorPositionX(),
                    er.move.ypos - mouse.GetCursorPositionY()
                };
                mouse.Invoke(param);
            }
            else if(er.type == Type::Button)
            {
                MouseButton button;
                if(er.button.button == GLFW_MOUSE_BUTTON_LEFT)
                    button = MOUSE_LEFT;
                else if(er.button.button == GLFW_MOUSE_BUTTON_MIDDLE)
                    button = MOUSE_MIDDLE;
                else if(er.button.button == GLFW_MOUSE_BUTTON_RIGHT)
                    button = MOUSE_RIGHT;
                else
                    continue;

                if(er.button.action == GLFW_PRESS)
                    mouse.Invoke(MouseButtonDown{ button });
                else if(er.button.action == GLFW_RELEASE)
                    mouse.Invoke(MouseButtonUp{ button });
            }
            else if(er.type == Type::EnterLeave)
            {
                if(er.enterLeave.entered)
                    mouse.Invoke(CursorEnter{});
                else
                    mouse.Invoke(CursorLeave{});
            }
            else
            {
                AGZ_ASSERT(type == Type::Type::Scroll);
                mouse.Invoke(WheelScroll{ er.scroll.offset });
            }
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

    GLFWMouseCapturer::EventRecord rc;
    rc.type = GLFWMouseCapturer::Type::Move;
    rc.move = { xpos, ypos };
    it->second->eventRecords_.push_back(rc);
}

inline void GLFWMouseButtonCallback(GLFWwindow *window, int button, int action, [[maybe_unused]] int mods)
{
    auto it = Impl::GLFWWindow2MouseCapturer.find(window);
    if(it == Impl::GLFWWindow2MouseCapturer.end())
        return;

    GLFWMouseCapturer::EventRecord rc;
    rc.type = GLFWMouseCapturer::Type::Button;
    rc.button = { button, action };
    it->second->eventRecords_.push_back(rc);
}

inline void GLFWCursorEnterLeaveCallback(GLFWwindow *window, int entered)
{
    auto it = Impl::GLFWWindow2MouseCapturer.find(window);
    if(it == Impl::GLFWWindow2MouseCapturer.end())
        return;

    GLFWMouseCapturer::EventRecord rc;
    rc.type = GLFWMouseCapturer::Type::EnterLeave;
    rc.enterLeave = { entered != 0 };
    it->second->eventRecords_.push_back(rc);
}

inline void GLFWWheelScrollCallback(GLFWwindow *window, [[maybe_unused]] double xoffset, double yoffset)
{
    auto it = Impl::GLFWWindow2MouseCapturer.find(window);
    if(it == Impl::GLFWWindow2MouseCapturer.end())
        return;

    GLFWMouseCapturer::EventRecord rc;
    rc.type = GLFWMouseCapturer::Type::Scroll;
    rc.scroll = { yoffset };
    it->second->eventRecords_.push_back(rc);
}

/**
 * @endcond
 */

} // namespace AGZ::Input

#endif // #ifdef AGZ_USE_GLFW
