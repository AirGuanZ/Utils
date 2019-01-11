#pragma once

// include glfw before this header

#ifdef AGZ_USE_GLFW

#include <unordered_map>
#include <vector>

#include "Keyboard.h"

namespace AGZ::Input
{

class GLFWKeyboardCapturer;

/**
 * @cond
 */

namespace Impl
{
    inline std::unordered_map<GLFWwindow*, GLFWKeyboardCapturer*> GLFWWindow2KeyboardCapturer;
}

inline void GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

/**
 * @endcond
 */

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
 * @cond
 */

inline void GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto it = Impl::GLFWWindow2KeyboardCapturer.find(window);
    if(it == Impl::GLFWWindow2KeyboardCapturer.end())
        return;

    auto cpr = it->second;
    cpr->AddEventRecord(key, action);
}

/**
 * @endcond
 */

} // namespace AGZ::Input

#endif // #ifdef AGZ_USE_GLFW
