#pragma once

#ifdef AGZ_USE_OPENGL

#include <vector>

#include "../../../Utils/FileSys.h"
#include "../../../Utils/Misc.h"
#include "UniformVariable.h"

namespace AGZ::OpenGL
{
    
template<GLenum ShaderType>
class TShader : public Uncopiable
{
    GLuint handle_ = 0;
    Str8 errMsg_;

public:

    ~TShader()
    {
        Destroy();
    }

    void Destroy() noexcept
    {
        if(!handle_)
            return;
        glDeleteShader(handle_);
        handle_ = 0;
    }

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
        GLint lenSrc = GLint(src.Length());
        glShaderSource(newHandle, 1, &charSrc, &lenSrc);

        GLint result;
        glCompileShader(newHandle);
        glGetShaderiv(newHandle, GL_COMPILE_STATUS, &result);
        if(result != GL_TRUE)
        {
            GLint logLen;
            glGetShaderiv(newHandle, GL_INFO_LOG_LENGTH, &logLen);

            std::vector<char> logBuf(logLen + 1);
            glGetShaderInfoLog(newHandle, logLen + 1, nullptr, log.data());
            errMsg_ = log.data();

            return false;
        }

        handle_ = newHandle;
        newHandleGuard.Dismiss();

        return true;
    }

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

    const Str8 &GetErrMsg() const noexcept
    {
        return errMsg_;
    }
};

using VertexShader   = TShader<GL_VERTEX_SHADER>;
using FragmentShader = TShader<GL_FRAGMENT_SHADER>;

AGZ_NEW_EXCEPTION(UniformVariableTypeError);
AGZ_NEW_EXCEPTION(UniformVariableNameError);

class Program
{
    GLuint handle_;

    explicit Program(GLuint handle) noexcept
        : handle_(handle)
    {
        AGZ_ASSERT(handle && glIsProgram(handle));
    }

public:

    template<typename...VarTypes>
    UniformVariable<VarTypes...> GetUniformVariable(const char *name) const
    {
        GLuint index; GLenum type;
        glGetUniformIndices(handle_, 1, &name, &index);
        if(index == GL_INVALID_INDEX)
            throw UniformVariableNameError(name);

        glGetActiveUniform(handle_, index, 0, nullptr, nullptr, &type, nullptr);

        if(type != Impl::Var2GL<VarTypes...>::Value)
            throw UniformVariableTypeError(name);

        return UniformVariable<VarTypes...>(glGetUniformLocation(handle_, name));
    }

    void Bind() const noexcept
    {
        glUseProgram(handle_);
    }

    void Unbind() const noexcept
    {
#       if defined(_DEBUG) || defined(DEBUG)
        GLint cur;
        glGetIntegerv(GL_CURRENT_PROGRAM, &cur);
        AGZ_ASSERT(cur == GLint(handle_));
#       endif
        glUseProgram(0);
    }
};

class ProgramBuilder
{
    static constexpr int MAX_SHADER_COUNT = 4;

    GLuint shaderHandles_[MAX_SHADER_COUNT];
    int shaderCount_;

    bool AddShaderImpl(GLuint shader) noexcept
    {
        if(shaderCount_ >= MAX_SHADER_COUNT)
            return false;
        shaderHandles_[shaderCount_++] = shader;
        return true;
    }

    bool AddShaderImpl(GLuint shader0, GLuint shader1, GLuint...otherShaders) noexcept
    {
        return AddShaderImpl(shader0) && AddShaderImpl(shader1, otherShaders...);
    }

public:

    explicit ProgramBuilder(GLuint...shaders) noexcept
        : shaderCount_(0)
    {
        AddShader(shaders...);
    }

    bool AddShader(GLuint...shaders) noexcept
    {
        int oldShaderCount = shaderCount_;
        if(!AddShaderImpl(shaders...))
        {
            shaderCount_ = oldShaderCount;
            return false;
        }
        return true;
    }

    int GetShaderCount() const noexcept
    {
        return shaderCount_;
    }

    void Clear() noexcept
    {
        shaderCount_ = 0;
    }
};

} // namespace AGZ::OpenGL

#endif // #ifdef AGZ_USE_OPENGL
