#pragma once

#include "../../Utils/FileSys.h"
#include "AttribVariable.h"
#include "UniformVariable.h"

namespace AGZ::GraphicsAPI::GL
{

AGZ_NEW_EXCEPTION(AttribVariableTypeException,  Exception);
AGZ_NEW_EXCEPTION(AttribVariableNameException,  Exception);
AGZ_NEW_EXCEPTION(UniformVariableTypeException, Exception);
AGZ_NEW_EXCEPTION(UniformVariableNameException, Exception);
AGZ_NEW_EXCEPTION(UniformBlockSizeException,    Exception);
AGZ_NEW_EXCEPTION(UniformBlockNameException,    Exception);

/**
 * @brief 完整的着色器程序
 */
class Program : public GLObject
{
    friend class ProgramBuilder;

    explicit Program(GLuint handle) noexcept
        : GLObject(handle)
    {
        AGZ_ASSERT(handle && AGZ_GL_CTX glIsProgram(handle));
    }

public:

    Program() noexcept
        : GLObject(0)
    {
        
    }

    Program(Program &&moveFrom) noexcept
        : GLObject(moveFrom.handle_)
    {
        moveFrom.handle_ = 0;
    }

    Program &operator=(Program &&moveFrom) noexcept
    {
        Destroy();
        std::swap(handle_, moveFrom.handle_);
        return *this;
    }

    /** 析构时将Program Object标记为删除 */
    ~Program()
    {
        Destroy();
    }

    /** 将Program Object标记为删除 */
    void Destroy() noexcept
    {
        if(handle_)
        {
            AGZ_GL_CTX glDeleteProgram(handle_);
            handle_ = 0;
        }
    }

    /**
     * @brief 取得具有指定类型和名字的Uniform Variable
     * 
     * @tparam VarType 变量类型
     * @param name 变量名
     * 
     * @exception UniformVariableNameException 找不到名为 name 的uniform变量时抛出
     * @exception UniformVariableTypeException 变量类型与Shader中的不匹配时抛出
     */
    template<typename VarType>
    UniformVariable<VarType> GetUniformVariable(const char *name) const
    {
        AGZ_ASSERT(handle_);

        GLuint index;
        AGZ_GL_CTX glGetUniformIndices(handle_, 1, &name, &index);
        if(index == GL_INVALID_INDEX)
            throw UniformVariableNameException(std::string("Invalid uniform variable name: ") + name);
        
        GLint size;  GLenum type;
        AGZ_GL_CTX glGetActiveUniform(handle_, index, 0, nullptr, &size, &type, nullptr);
        if(type != Impl::Var2GL<VarType>::Type)
            throw UniformVariableTypeException(std::string("Invalid uniform variable type of ") + name);

        return UniformVariable<VarType>(AGZ_GL_CTX glGetUniformLocation(handle_, name));
    }

    template<typename BlockType>
    Std140UniformBlock<BlockType> GetStd140UniformBlock(const char *name) const
    {
        AGZ_ASSERT(handle_);

        GLuint blockIndex = AGZ_GL_CTX glGetUniformBlockIndex(handle_, name);
        if(blockIndex == GL_INVALID_INDEX)
            throw UniformBlockNameException(std::string("Invalid uniform block name: ") + name);

        GLint size;
        AGZ_GL_CTX glGetActiveUniformBlockiv(handle_, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
        if(sizeof(BlockType) != size)
            throw UniformBlockSizeException(std::string("Invalid uniform block size of ") + name);

        return Std140UniformBlock<BlockType>(handle_, blockIndex);
    }

    /**
     * @brief 取得具有指定类型和名字的Attrib Variable
     *
     * @tparam VarType 变量类型
     * @param name 变量名
     *
     * @exception AttribVariableNameException 找不到名为 name 的属性时抛出
     * @exception AttribVariableTypeException 属性类型与Shader中的不匹配时抛出
     */
    template<typename VarType>
    AttribVariable<VarType> GetAttribVariable(const char *name) const
    {
        AGZ_ASSERT(handle_);

        GLint loc = AGZ_GL_CTX glGetAttribLocation(handle_, name);
        if(loc < 0)
            throw AttribVariableNameException(std::string("Invalid attrib variable name: ") + name);

        GLint size;  GLenum type;
        AGZ_GL_CTX glGetActiveAttrib(handle_, loc, 0, nullptr, &size, &type, nullptr);
        if(type != Impl::Var2GL<VarType>::Type)
            throw AttribVariableTypeException(std::string("Invalid attrib variable type of ") + name);

        return AttribVariable<VarType>(loc);
    }

    /**
     * @brief 将该程序绑定到着色管线
     */
    void Bind() const noexcept
    {
        AGZ_ASSERT(handle_);
        AGZ_GL_CTX glUseProgram(handle_);
    }

    /**
     * @brief 解除该程序和着色管线间的绑定
     */
    void Unbind() const noexcept
    {
        AGZ_ASSERT(handle_);
#       if defined(_DEBUG) || defined(DEBUG)
        GLint cur;
        AGZ_GL_CTX glGetIntegerv(GL_CURRENT_PROGRAM, &cur);
        AGZ_ASSERT(cur == GLint(handle_));
#       endif
        AGZ_GL_CTX glUseProgram(0);
    }
};

} // namespace AGZ::GraphicsAPI::GL
