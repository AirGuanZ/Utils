#pragma once

#include <vector>

#include <AGZUtils/Misc/ScopeGuard.h>
#include <AGZUtils/Misc/Uncopiable.h>
#include <AGZUtils/FileSys/Raw.h>

#include "Common.h"

namespace AGZ::GL
{

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
    Str8 errMsg_;

public:

    TShader() = default;

    TShader(TShader<ShaderType> &&moveFrom) noexcept
        : GLObject(moveFrom.handle_), errMsg_(std::move(moveFrom.errMsg_))
    {
        moveFrom.handle_ = 0;
    }

    TShader<ShaderType> &operator=(TShader<ShaderType> &&moveFrom) noexcept
    {
        Destroy();
        std::swap(handle_, moveFrom.handle_);
        errMsg_ = std::move(moveFrom.errMsg_);
        return *this;
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
     */
    bool LoadFromMemory(const Str8 &src)
    {
        Destroy();
        
        GLuint newHandle = glCreateShader(ShaderType);
        ScopeGuard newHandleGuard([=]() { glDeleteShader(newHandle); });

        if(!newHandle)
        {
            errMsg_ = "Failed to create shader object";
            return false;
        }

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
            errMsg_ = logBuf.data();

            return false;
        }

        handle_ = newHandle;
        newHandleGuard.Dismiss();

        return true;
    }

    /**
     * @brief 从文件中加载Shader
     * @note 原Shader会被标记为删除
     */
    bool LoadFromFile(const Str8 &filename)
    {
        Destroy();

        Str8 src;
        if(!FileSys::ReadTextFileRaw(filename, &src))
        {
            errMsg_ = "Failed to load file content from " + filename;
            return false;
        }

        return LoadFromMemory(src);
    }

    /**
     * @brief 取得上一条错误信息
     */
    const Str8 &GetErrMsg() const noexcept
    {
        return errMsg_;
    }
};

using VertexShader   = TShader<GL_VERTEX_SHADER>;
using FragmentShader = TShader<GL_FRAGMENT_SHADER>;

} // namespace AGZ::GL
