#pragma once

#include "RenderBuffer.h"
#include "Texture2D.h"

namespace AGZ::GraphicsAPI::GL
{
    
class FrameBuffer : public GLObject
{
public:

    /**
 * @param initHandle 是否立即创建一个GL FrameBuffer Name
 */
    explicit FrameBuffer(bool initHandle = false) noexcept
        : GLObject(0)
    {
        if(initHandle)
            InitializeHandle();
    }

    FrameBuffer(FrameBuffer &&moveFrom) noexcept
        : GLObject(moveFrom.handle_)
    {
        moveFrom.handle_ = 0;
    }

    FrameBuffer &operator=(FrameBuffer &&moveFrom) noexcept
    {
        Destroy();
        std::swap(handle_, moveFrom.handle_);
        return *this;
    }

    ~FrameBuffer()
    {
        Destroy();
    }

    /**
     * @brief 若含有FrameBuffer Object，将该Buffer标记为删除
     */
    void Destroy() noexcept
    {
        if(handle_)
        {
            glDeleteFramebuffers(1, &handle_);
            handle_ = 0;
        }
    }

    /**
     * @brief 创建一个GL FrameBuffer Name
     * @note 若已创建一个Name且未经删除，则调用该函数的结果是未定义的
     */
    void InitializeHandle() noexcept
    {
        AGZ_ASSERT(!handle_);
        glCreateFramebuffers(1, &handle_);
    }

    /**
     * @brief 连接一块Texture2D
     * 见 glNamedFramebufferTexture
     */
    void Attach(GLenum attachment, const Texture2D &tex) noexcept
    {
        glNamedFramebufferTexture(handle_, attachment, tex.GetHandle(), 0);
    }

    /**
     * @brief 连接一块RenderBuffer
     * 见 glNamedFramebufferRenderbuffer
     */
    void Attach(GLenum attachment, const RenderBuffer &buffer) noexcept
    {
        AGZ_ASSERT(handle_ && buffer.GetHandle());
        glNamedFramebufferRenderbuffer(handle_, attachment, GL_RENDERBUFFER, buffer.GetHandle());
    }

    /**
     * @brief 绑定为当前使用的FrameBuffer
     */
    void Bind() const noexcept
    {
        AGZ_ASSERT(handle_);
        glBindFramebuffer(GL_FRAMEBUFFER, handle_);
    }

    /**
     * @brief 作为当前使用的FrameBuffer解除绑定
     */
    void Unbind() const noexcept
    {
        AGZ_ASSERT(handle_);
#       if defined(_DEBUG) || defined(DEBUG)
        GLint cur;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &cur);
        AGZ_ASSERT(cur == GLint(handle_));
#       endif
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    /**
     * @brief 是检查作为framebuffer的completeness
     */
    bool IsComplete() const noexcept
    {
        AGZ_ASSERT(handle_);
        return glCheckNamedFramebufferStatus(handle_, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }
};

} // namespace AGZ::GraphicsAPI::GL
