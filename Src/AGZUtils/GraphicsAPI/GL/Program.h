#pragma once

#include "../../Utils/FileSys.h"
#include "AttribVariable.h"
#include "UniformVariable.h"

namespace AGZ::GL
{

AGZ_NEW_EXCEPTION(AttribVariableTypeException);
AGZ_NEW_EXCEPTION(AttribVariableNameException);
AGZ_NEW_EXCEPTION(UniformVariableTypeException);
AGZ_NEW_EXCEPTION(UniformVariableNameException);
AGZ_NEW_EXCEPTION(UniformBlockSizeException);
AGZ_NEW_EXCEPTION(UniformBlockNameException);

/**
 * @brief 完整的着色器程序
 */
class Program : public GLObject
{
    friend class ProgramBuilder;

    explicit Program(GLuint handle) noexcept
        : GLObject(handle)
    {
        AGZ_ASSERT(handle && glIsProgram(handle));
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
            glDeleteProgram(handle_);
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
        glGetUniformIndices(handle_, 1, &name, &index);
        if(index == GL_INVALID_INDEX)
            throw UniformVariableNameException(name);
        
        GLint size;  GLenum type;
        glGetActiveUniform(handle_, index, 0, nullptr, &size, &type, nullptr);
        if(type != Impl::Var2GL<VarType>::Type)
            throw UniformVariableTypeException(name);

        return UniformVariable<VarType>(glGetUniformLocation(handle_, name));
    }

    template<typename BlockType>
    Std140UniformBlock<BlockType> GetStd140UniformBlock(const char *name) const
    {
        AGZ_ASSERT(handle_);

        GLuint blockIndex = glGetUniformBlockIndex(handle_, name);
        if(blockIndex == GL_INVALID_INDEX)
            throw UniformBlockNameException(name);

        GLint size;
        glGetActiveUniformBlockiv(handle_, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
        if(sizeof(BlockType) != size)
            throw UniformBlockSizeException(name);

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

        GLint loc = glGetAttribLocation(handle_, name);
        if(loc < 0)
            throw AttribVariableNameException(name);

        GLint size;  GLenum type;
        glGetActiveAttrib(handle_, loc, 0, nullptr, &size, &type, nullptr);
        if(type != Impl::Var2GL<VarType>::Type)
            throw AttribVariableTypeException(name);

        return AttribVariable<VarType>(loc);
    }

    /**
     * @brief 将该程序绑定到着色管线
     */
    void Bind() const noexcept
    {
        AGZ_ASSERT(handle_);
        glUseProgram(handle_);
    }

    /**
     * @brief 解除该程序和着色管线间的绑定
     */
    void Unbind() const noexcept
    {
        AGZ_ASSERT(handle_);
#       if defined(_DEBUG) || defined(DEBUG)
        GLint cur;
        glGetIntegerv(GL_CURRENT_PROGRAM, &cur);
        AGZ_ASSERT(cur == GLint(handle_));
#       endif
        glUseProgram(0);
    }
};

} // namespace AGZ::GL
