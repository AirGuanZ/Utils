#pragma once

#include "Common.h"

namespace AGZ::GraphicsAPI::GL
{

/**
 * @brief 对常用gl*函数的直接封装
 */
class RenderContext
{
public:

    /**
     * @brief 设置背景颜色
     */
    static void SetClearColor(const Vec4f &color) noexcept
    {
        AGZ_GL_CTX glClearColor(color.r, color.g, color.b, color.a);
    }

    /**
     * @brief 清空background color buffer为背景颜色
     */
    static void ClearColor() noexcept
    {
        AGZ_GL_CTX glClear(GL_COLOR_BUFFER_BIT);
    }

    /**
     * @brief 清空background depth buffer
     */
    static void ClearDepth() noexcept
    {
        AGZ_GL_CTX glClear(GL_DEPTH_BUFFER_BIT);
    }

    /**
     * @brief ClearColor & ClearDepth
     */
    static void ClearColorAndDepth() noexcept
    {
        AGZ_GL_CTX glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    /**
     * @brief 从当前绑定的vertex buffer取顶点数据进行绘制
     */
    static void DrawVertices(GLenum primMode, uint32_t begin, uint32_t count) noexcept
    {
        AGZ_GL_CTX glDrawArrays(primMode, GLint(begin), GLsizei(count));
    }

    /**
     * @brief 从当前绑定的element buffer取顶点索引数据进行绘制
     */
    static void DrawElements(GLenum primMode, uint32_t begin, uint32_t count, GLenum elemType) noexcept
    {
        AGZ_ASSERT(elemType == GL_UNSIGNED_BYTE || elemType == GL_UNSIGNED_SHORT || elemType == GL_UNSIGNED_INT);
        size_t elemSize;
        switch(elemType)
        {
        case GL_UNSIGNED_BYTE:  elemSize = sizeof(GLubyte);  break;
        case GL_UNSIGNED_SHORT: elemSize = sizeof(GLushort); break;
        case GL_UNSIGNED_INT:   elemSize = sizeof(GLuint);   break;
        default: Unreachable();
        }
        AGZ_GL_CTX glDrawElements(primMode, count, elemType, reinterpret_cast<void*>(elemSize * size_t(begin)));
    }

    /**
     * @brief 启用深度测试
     */
    static void EnableDepthTest() noexcept
    {
        AGZ_GL_CTX glEnable(GL_DEPTH_TEST);
    }

    /**
     * @brief 禁用深度测试
     */
    static void DisableDepthTest() noexcept
    {
        AGZ_GL_CTX glDisable(GL_DEPTH_TEST);
    }

    /**
     * @brief 是否启用了深度测试
     */
    static bool IsDepthTestEnabled() noexcept
    {
        return AGZ_GL_CTX glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;
    }

    /**
     * @brief 设置多边形填充模式：GL_LINE, GL_FILL, GL_POINT
     */
    static void SetFillMode(GLenum fillMode) noexcept
    {
        AGZ_GL_CTX glPolygonMode(GL_FRONT_AND_BACK, fillMode);
    }

    /**
     * @brief 取得当前的多边形填充模式
     */
    static GLenum GetFillMode() noexcept
    {
        GLint ret;
        AGZ_GL_CTX glGetIntegerv(GL_POLYGON_MODE, &ret);
        return GLenum(ret);
    }
};
    
} // namespace AGZ::GraphicsAPI::GL
