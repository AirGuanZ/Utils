#pragma once

#include "Common.h"

namespace AGZ::GraphicsAPI::GL
{
    
/**
 * @brief 对OpenGL RenderBuffer的直接封装
 * 见 https://www.khronos.org/opengl/wiki/Renderbuffer_Object
 */
class RenderBuffer : public GLObject
{
public:

    /**
     * @param initHandle 是否立即创建一个GL RenderBuffer Name
     */
    explicit RenderBuffer(bool initHandle = false) noexcept
        : GLObject(0)
    {
        if(initHandle)
            InitializeHandle();
    }

    RenderBuffer(RenderBuffer &&moveFrom) noexcept
        : GLObject(moveFrom.handle_)
    {
        moveFrom.handle_ = 0;
    }

    RenderBuffer &operator=(RenderBuffer &&moveFrom) noexcept
    {
        Destroy();
        std::swap(handle_, moveFrom.handle_);
        return *this;
    }

    ~RenderBuffer()
    {
        Destroy();
    }

    /**
     * @brief 若含有RenderBuffer Object，将该Buffer标记为删除
     */
    void Destroy() noexcept
    {
        if(handle_)
        {
            AGZ_GL_CTX glDeleteRenderbuffers(1, &handle_);
            handle_ = 0;
        }
    }

    /**
     * @brief 创建一个GL RenderBuffer Name
     * @note 若已创建一个Name且未经删除，则调用该函数的结果是未定义的
     */
    void InitializeHandle() noexcept
    {
        AGZ_ASSERT(!handle_);
        AGZ_GL_CTX glCreateRenderbuffers(1, &handle_);
    }

    /**
     * @brief 设置该RenderBuffer内部的格式和大小
     * 见 glNamedRenderbufferStorage
     * @note 在使用该对象前至少调用一次
     * @note 可重复调用，重复时会清除以前设置的格式、大小和内容
     */
    void SetFormat(GLsizei width, GLsizei height, GLenum internalFormat) const noexcept
    {
		AGZ_ASSERT(handle_);
        AGZ_GL_CTX glNamedRenderbufferStorage(handle_, internalFormat, width, height);
    }
};

} // namespace AGZ::GraphicsAPI::GL
