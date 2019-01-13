#pragma once

#include "Common.h"

namespace AGZ::GL
{

namespace Impl
{
    template<typename DataPixel> struct PT2DT;

    template<> struct PT2DT<GLfloat> { static constexpr GLenum format = GL_RED;  static constexpr GLenum type = GL_FLOAT;         };
    template<> struct PT2DT<Vec3f>   { static constexpr GLenum format = GL_RGB;  static constexpr GLenum type = GL_FLOAT;         };
    template<> struct PT2DT<Vec4f>   { static constexpr GLenum format = GL_RGBA; static constexpr GLenum type = GL_FLOAT;         };
    template<> struct PT2DT<GLubyte> { static constexpr GLenum format = GL_RED;  static constexpr GLenum type = GL_UNSIGNED_BYTE; };
    template<> struct PT2DT<Vec3b>   { static constexpr GLenum format = GL_RGB;  static constexpr GLenum type = GL_UNSIGNED_BYTE; };
    template<> struct PT2DT<Vec4b>   { static constexpr GLenum format = GL_RGBA; static constexpr GLenum type = GL_UNSIGNED_BYTE; };
}

/**
 * @brief 对OpenGL二维纹理对象的封装
 */
class Texture2D : public GLObject
{
public:


    /**
     * @param initHandle 是否立即创建一个GL Texture2D Name
     */
    explicit Texture2D(bool initHandle = false) noexcept
    {
        if(initHandle)
            InitializeHandle();
    }

    Texture2D(Texture2D &&moveFrom) noexcept
        : GLObject(moveFrom.handle_)
    {
        moveFrom.handle_ = 0;
    }

    Texture2D &operator=(Texture2D &&moveFrom) noexcept
    {
        Destroy();
        std::swap(handle_, moveFrom.handle_);
        return *this;
    }

    ~Texture2D()
    {
        Destroy();
    }

    /**
     * @brief 若含有Texture2D Object，将该Buffer标记为删除
     */
    void Destroy() noexcept
    {
        if(handle_)
        {
            glDeleteTextures(1, &handle_);
            handle_ = 0;
        }
    }

    /**
     * @brief 创建一个GL Texture2D Name
     * @note 若已创建一个Name且未经删除，则调用该函数的结果是未定义的
     */
    void InitializeHandle() noexcept
    {
        AGZ_ASSERT(!handle_);
        glCreateTextures(GL_TEXTURE_2D, 1, &handle_);
    }

    /**
     * @brief 初始化该纹理对象的格式和内容
     * 
     * 相当于InitializeFormat + ReinitializeData一次完成
     * 
     * @param levels MipMap级数
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param internalFormat 纹理内部像素格式，参见 glTextureStorage2D
     * @param data 用于初始化的数据
     */
    template<typename DataTexelType>
    void InitializeFormatAndData(
        GLsizei levels, GLsizei width, GLsizei height, GLenum internalFormat, const DataTexelType *data) const noexcept
    {
        AGZ_ASSERT(handle_);
        glTextureStorage2D(handle_, levels, internalFormat, width, height);
        glTextureSubImage2D(
            handle_, 0, 0, 0, width, height,
            Impl::PT2DT<DataTexelType>::format, Impl::PT2DT<DataTexelType>::type, data);
        glGenerateTextureMipmap(handle_);
    }

    /**
     * @brief 初始化内部像素格式，除非被Destroy，否则只能调用一次
     * @param levels MipMap级数
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param internalFormat 纹理内部像素格式，参见 glTextureStorage2D
     */
    void InitializeFormat(GLsizei levels, GLsizei width, GLsizei height, GLenum internalFormat) const noexcept
    {
        AGZ_ASSERT(handle_);
        glTextureStorage2D(handle_, levels, internalFormat, width, height);
    }

    /**
     * @brief 重新设置纹理内容
     * 
     * 只能在InitializeFormatAndData或InitializeFormat之后调用，可重复调用多次
     * 
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param data 用于初始化的数据
     */
    template<typename DataTexelType>
    void ReinitializeData(GLsizei width, GLsizei height, const DataTexelType *data) const noexcept
    {
        AGZ_ASSERT(handle_);
        glTextureSubImage2D(
            handle_, 0, 0, 0, width, height,
            Impl::PT2DT<DataTexelType>::format, Impl::PT2DT<DataTexelType>::type, data);
        glGenerateTextureMipmap(handle_);
    }

    /**
     * @brief 设置纹理参数，见 glTextureParameteri
     */
    void SetParameter(GLenum paramName, GLint paramValue) const noexcept
    {
        AGZ_ASSERT(handle_);
        glTextureParameteri(handle_, paramName, paramValue);
    }

    /**
     * @brief 绑定到指定的Texture Unit
     */
    void Bind(GLuint textureUnit) const noexcept
    {
        AGZ_ASSERT(handle_);
        glBindTextureUnit(textureUnit, handle_);
    }

    /**
     * @brief 从指定的Texture Unit解绑
     */
    void Unbind(GLuint textureUnit) const noexcept
    {
        AGZ_ASSERT(handle_);
        glBindTextureUnit(textureUnit, 0);
    }
};

} // namespace AGZ::GL
