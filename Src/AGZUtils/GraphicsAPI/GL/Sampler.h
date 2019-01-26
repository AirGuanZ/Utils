#pragma once

#include "Common.h"

namespace AGZ::GraphicsAPI::GL
{

/**
 * @brief 对GL Sampler Object的直接封装
 */
class Sampler : public GLObject
{
public:

    /**
     * @param initHandle 是否立即创建一个GL Buffer Name
     */
    explicit Sampler(bool initHandle = true) noexcept
    {
        if(initHandle)
            InitializeHandle();
    }

    Sampler(Sampler &&moveFrom) noexcept
        : GLObject(moveFrom.handle_)
    {
        moveFrom.handle_ = 0;
    }

    Sampler &operator=(Sampler &&moveFrom) noexcept
    {
        Destroy();
        std::swap(handle_, moveFrom.handle_);
        return *this;
    }

    ~Sampler()
    {
        Destroy();
    }

    /**
     * @brief 若含有Sampler Object，将该Sampler标记为删除
     */
    void Destroy() noexcept
    {
        if(handle_)
        {
            glDeleteSamplers(1, &handle_);
            handle_ = 0;
        }
    }

    /**
     * @brief 创建一个GL Sampler Name
     * @note 若已创建一个Name且未经删除，则调用该函数的结果是未定义的
     */
    void InitializeHandle() noexcept
    {
        AGZ_ASSERT(!handle_);
        glCreateSamplers(1, &handle_);
    }

    /**
     * @brief 见 glSamplerParameteri
     */
    void SetParameter(GLenum paramName, GLint paramValue) const noexcept
    {
        AGZ_ASSERT(handle_);
        glSamplerParameteri(handle_, paramName, paramValue);
    }

    /**
     * @brief 将该Sampler Object绑定到指定的Texture Unit
     */
    void Bind(GLuint textureUnit) const noexcept
    {
        AGZ_ASSERT(handle_);
        glBindSampler(textureUnit, handle_);
    }

    /**
     * @brief 将该Sampler Object从指定的Texture Unit解绑
     */
    void Unbind(GLuint textureUnit) const noexcept
    {
        AGZ_ASSERT(handle_);
        glBindSampler(textureUnit, 0);
    }
};
   
} // namespace AGZ::GraphicsAPI::GL
