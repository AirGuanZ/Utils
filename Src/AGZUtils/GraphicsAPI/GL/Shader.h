#pragma once

#include <vector>

#include "../../Misc/ScopeGuard.h"
#include "../../Utils/FileSys.h"
#include "Common.h"

namespace AGZ::GraphicsAPI::GL
{

AGZ_NEW_EXCEPTION(ShaderLoadingException, Exception);

/**
 * @brief 可编程管线某一阶段的Shader Object
 * @tparam ShaderType 着色器类型，取值范围为：
 *      - GL_COMPUTE_SHADER
 *      - GL_VERTEX_SHADER
 *      - GL_TESS_CONTROL_SHADER
 *      - GL_TESS_EVALUATION_SHADER
 *      - GL_GEOMETRY_SHADER
 *      - GL_FRAGMENT_SHADER
 */
template<GLenum ShaderType>
class TShader : public GLObject
{
public:

    TShader() = default;

    TShader(TShader<ShaderType> &&moveFrom) noexcept
        : GLObject(moveFrom.handle_)
    {
        moveFrom.handle_ = 0;
    }

    TShader<ShaderType> &operator=(TShader<ShaderType> &&moveFrom) noexcept
    {
        Destroy();
        std::swap(handle_, moveFrom.handle_);
        return *this;
    }

    /**
     * @brief 创建一个shader对象并从源代码中编译
     * @exception ShaderLoadingException 着色器创建/编译失败时抛出
     */
    static TShader<ShaderType> FromMemory(const Str8 &src)
    {
        TShader<ShaderType> ret;
        ret.LoadFromMemory(src);
        return ret;
    }

    /**
     * @brief 创建一个shader对象并从文件中编译
     * @exception ShaderLoadingException 着色器创建/编译失败时抛出
     */
    static TShader<ShaderType> FromFile(const Str8 &filename)
    {
        TShader<ShaderType> ret;
        ret.LoadFromFile(filename);
        return ret;
    }

    /** 销毁时将ShaderObject标记为删除 */
    ~TShader()
    {
        Destroy();
    }

    /**
     * @brief 删除Shader Object
     */
    void Destroy() noexcept
    {
        if(!handle_)
            return;
        glDeleteShader(handle_);
        handle_ = 0;
    }

    /**
     * @brief 从源代码中创建Shader
     * @note 原Shader会被标记为删除
     * @exception ShaderLoadingException 着色器加载失败时抛出
     */
    void LoadFromMemory(const Str8 &src)
    {
        Destroy();
        
        GLuint newHandle = glCreateShader(ShaderType);
        ScopeGuard newHandleGuard([=]() { glDeleteShader(newHandle); });

        if(!newHandle)
            throw ShaderLoadingException("Failed to create shader object");

        const char *charSrc = src.Data();
        auto lenSrc = static_cast<GLint>(src.Length());
        glShaderSource(newHandle, 1, &charSrc, &lenSrc);

        GLint result;
        glCompileShader(newHandle);
        glGetShaderiv(newHandle, GL_COMPILE_STATUS, &result);
        if(result != GL_TRUE)
        {
            GLint logLen;
            glGetShaderiv(newHandle, GL_INFO_LOG_LENGTH, &logLen);

            std::vector<char> logBuf(logLen + 1);
            glGetShaderInfoLog(newHandle, logLen + 1, nullptr, logBuf.data());

            throw ShaderLoadingException(logBuf.data());
        }

        handle_ = newHandle;
        newHandleGuard.Dismiss();
    }

    /**
     * @brief 从文件中加载Shader
     * @note 原Shader会被标记为删除
     * @exception ShaderLoadingException 着色器加载失败时抛出
     */
    void LoadFromFile(const Str8 &filename)
    {
        Destroy();

        Str8 src;
        if(!FileSys::ReadTextFileRaw(filename, &src))
            throw ShaderLoadingException("Failed to load file content from " + filename);
        LoadFromMemory(src);
    }
};

using VertexShader   = TShader<GL_VERTEX_SHADER>;
using FragmentShader = TShader<GL_FRAGMENT_SHADER>;

} // namespace AGZ::GraphicsAPI::GL
