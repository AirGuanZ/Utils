#pragma once

#include <vector>

#include "../../Misc/ScopeGuard.h"

#include "Program.h"
#include "Shader.h"

namespace AGZ::GraphicsAPI::GL
{

AGZ_NEW_EXCEPTION(ProgramBuilderTooMuchShaderException, Exception);
AGZ_NEW_EXCEPTION(ProgramBuilderProgramCreationException, Exception);
AGZ_NEW_EXCEPTION(ProgramBuilderLinkFailureException, Exception);

/**
 * @brief 用于将一系列Shader统合在一起链接成一个Program
 */
class ProgramBuilder
{
    static constexpr int MAX_SHADER_COUNT = 4;

    GLuint shaderHandles_[MAX_SHADER_COUNT];
    int shaderCount_;

    template<GLenum ShaderType>
    void AddShaderImpl(const TShader<ShaderType> &shader)
    {
        if(shaderCount_ >= MAX_SHADER_COUNT)
            throw ProgramBuilderTooMuchShaderException(
                "More than " + std::to_string(MAX_SHADER_COUNT) + " shaders");
        shaderHandles_[shaderCount_++] = shader.GetHandle();
    }

    template<GLenum ShaderType1, GLenum ShaderType2, GLenum...OtherShaderTypes>
    void AddShaderImpl(
        const TShader<ShaderType1> &shader0, const TShader<ShaderType2> &shader1,
        const TShader<OtherShaderTypes> &...otherShaders)
    {
        AddShaderImpl(shader0);
        AddShaderImpl(shader1, otherShaders...);
    }

public:

    /**
     * @brief 默认初始化为不带任何Shader的空Builder
     */
    ProgramBuilder() noexcept
        : shaderCount_(0)
    {
        
    }

    /**
     * @brief 以给定的一些Shader初始化该Builder，不占据ShaderObject的所有权
     * @exception ProgramBuilderTooMuchShaderException Shader数量超过上限时抛出
     */
    template<GLenum ShaderType, GLenum...OtherShaderTypes>
    explicit ProgramBuilder(
        const TShader<ShaderType> &shader1, const TShader<OtherShaderTypes>&...otherShaders)
        : ProgramBuilder()
    {
        AddShader(shader1, otherShaders...);
    }

    /**
     * @brief 直接创建一个Program并链接给定的Shader
     * @exception ProgramBuilderTooMuchShaderException Shader数量超过上限时抛出
     */
    template<GLenum...ShaderTypes>
    static Program BuildOnce(const TShader<ShaderTypes>&...shaders)
    {
        ProgramBuilder builder(shaders...);
        return builder.Build();
    }

    /**
     * @brief 添加更多的ShaderObject，不占据所有权
     * @exception ProgramBuilderTooMuchShaderException Shader数量超过上限时抛出
     */
    template<GLenum...ShaderTypes>
    void AddShader(const TShader<ShaderTypes>&...shaders)
    {
        int oldShaderCount = shaderCount_;
        try
        {
            AddShaderImpl(shaders...);
        }
        catch(...)
        {
            shaderCount_ = oldShaderCount;
            throw;
        }
    }

    /**
     * @brief 目前包含多少ShaderObject
     */
    int GetShaderCount() const noexcept
    {
        return shaderCount_;
    }

    /**
     * @brief 清空内部所有的ShaderObject
     */
    void Clear() noexcept
    {
        shaderCount_ = 0;
    }

    /**
     * @brief 用内部所有的ShaderObject创建一个ProgramObject并链接
     * @exception ProgramBuilderProgramCreationException 创建GL Program Object失败时抛出
     * @exception ProgramBuilderLinkFailureException 链接失败时抛出
     */
    Program Build() const
    {
        GLuint handle = glCreateProgram();
        if(!handle)
            throw ProgramBuilderProgramCreationException("Failed to create program object");
        ScopeGuard handleGuard([=]()
        {
            if(handle)
            {
                GLuint attachedShaders[MAX_SHADER_COUNT];
                GLsizei count;
                glGetAttachedShaders(handle, MAX_SHADER_COUNT, &count, attachedShaders);

                for(GLsizei i = 0; i < count; ++i)
                    glDetachShader(handle, attachedShaders[i]);

                glDeleteProgram(handle);
            }
        });
        
        for(int i = 0; i < shaderCount_; ++i)
            glAttachShader(handle, shaderHandles_[i]);

        glLinkProgram(handle);

        GLint result;
        glGetProgramiv(handle, GL_LINK_STATUS, &result);
        if(result != GL_TRUE)
        {
            GLint logLen;
            glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> logBuf(logLen + 1);
            glGetProgramInfoLog(handle, GLsizei(logBuf.size()), nullptr, logBuf.data());

            throw ProgramBuilderLinkFailureException(logBuf.data());
        }

        for(int i = 0; i < shaderCount_; ++i)
            glDetachShader(handle, shaderHandles_[i]);

        handleGuard.Dismiss();
        return Program(handle);
    }
};

} // namespace AGZ::GraphicsAPI::GL
