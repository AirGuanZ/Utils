#pragma once

#include <unordered_map>

#include "../../Utils/Alloc.h"
#include "Common.h"

namespace AGZ::GraphicsAPI::GL
{

/**
 * @brief Uniform Variable Location Wrapper
 * @tparam VarType GLSL变量在CPU一侧的对应类型
 */
template<typename VarType>
class UniformVariable
{
    friend class Program;

    GLint loc_;

    explicit UniformVariable(GLint loc) noexcept
        : loc_(loc)
    {

    }

public:

    UniformVariable() noexcept
        : loc_(0)
    {
        
    }

    /**
     * @brief 设置该location对应的uniform variable值
     */
    void BindValue(const VarType &var) const
    {
        Impl::SetUniform(loc_, var);
    }

    /**
     * @brief 取得loc值
     */
    GLint GetLocation() const noexcept
    {
        return loc_;
    }

    /**
     * @brief 重载小于运算符
     * @note 只有来自同一Program的UniformVariable间的比较才有意义
     */
    template<typename OtherVarType>
    bool operator<(UniformVariable<OtherVarType> rhs) const noexcept
    {
        return loc_ < rhs.GetLocation();
    }
};

/**
 * @brief 对layout(std140) uniform BlockName { ... }的block index的封装
 */
template<typename BlockType>
class Std140UniformBlock
{
    GLuint program_;
    GLuint idx_;

    friend class Program;

    Std140UniformBlock(GLuint program, GLuint idx) noexcept
        : program_(program), idx_(idx)
    {

    }

public:

    /**
     * @brief 取得该Uniform Block在Program中的编号
     */
    GLuint GetIndex() const noexcept
    {
        return idx_;
    }

    /**
     * @brief 将该Program中的该Block绑定到指定的UBO Binding Point
     */
    void Bind(GLuint bindingPoint) const noexcept
    {
        AGZ_GL_CTX glUniformBlockBinding(program_, idx_, bindingPoint);
    }
};

/**
 * @brief 一组UniformVariable-Value对，可统一进行绑定
 */
class UniformVariableAssignment
{
    class RecordInterface
    {
    public:

        virtual ~RecordInterface() = default;

        virtual void Bind() const noexcept = 0;

        virtual GLenum GetGLSLType() const noexcept = 0;

        virtual void SetValue(const void *value) noexcept = 0;
    };

    template<typename VarType>
    class UniformVariableRecord : public RecordInterface
    {
    public:

        UniformVariable<VarType> var;
        VarType value;

        UniformVariableRecord(UniformVariable<VarType> var, const VarType &value) noexcept
            : var(var), value(value)
        {
            
        }

        void Bind() const noexcept override
        {
            var.BindValue(value);
        }

        GLenum GetGLSLType() const noexcept override
        {
            return Impl::Var2GL<VarType>::Type;
        }

        void SetValue(const void *pValue) noexcept override
        {
            AGZ_ASSERT(pValue);
            value = *static_cast<const VarType*>(pValue);
        }
    };

    template<typename BlockType>
    struct Std140UniformBlockRecordValue
    {
        const Std140UniformBlockBuffer<BlockType> *buffer;
        GLuint bindingPoint;
    };

    template<typename BlockType>
    class Std140UniformBlockRecord : public RecordInterface
    {
    public:

        Std140UniformBlock<BlockType> block;
        const Std140UniformBlockBuffer<BlockType> *buffer;
        GLuint bindingPoint;

        Std140UniformBlockRecord(
            Std140UniformBlock<BlockType> block, const Std140UniformBlockBuffer<BlockType> *buffer, GLuint bindingPoint) noexcept
            : block(block), buffer(buffer), bindingPoint(bindingPoint)
        {
            
        }

        void Bind() const noexcept override
        {
            buffer->Bind(bindingPoint);
        }

        [[noreturn]] GLenum GetGLSLType() const noexcept override
        {
            std::terminate();
        }

        void SetValue(const void *value) noexcept override
        {
            auto tvalue = static_cast<const Std140UniformBlockRecordValue<BlockType>*>(value);
            buffer = tvalue->buffer;
            bindingPoint = tvalue->bindingPoint;
        }
    };

    ObjArena<> arena_;
    std::unordered_map<GLint, RecordInterface*> assignments_;

public:

    /**
     * @brief 添加/设置一个Variable-Value对
     */
    template<typename VarType>
    void SetValue(UniformVariable<VarType> var, const VarType &value)
    {
        auto it = assignments_.find(var.GetLocation());
        if(it != assignments_.end())
        {
            AGZ_ASSERT(it->second->GetGLSLType() == Impl::Var2GL<VarType>::Type);
            it->second->SetValue(&value);
        }
        else
        {
            auto *rc = arena_.Create<UniformVariableRecord<VarType>>(var, value);
            assignments_[var.GetLocation()] = rc;
        }
    }

    /**
     * @brief 添加/设置一个StdUniformBlock-UniformBlockBuffer对
     */
    template<typename BlockType>
    void SetValue(Std140UniformBlock<BlockType> block, const Std140UniformBlockBuffer<BlockType> *buffer, GLuint bindingPoint)
    {
        GLint key = static_cast<GLint>(block.GetIndex()) | 0xf00000;
        AGZ_ASSERT(static_cast<GLuint>(key) != block.GetIndex());
        auto it = assignments_.find(key);
        if(it != assignments_.end())
        {
            Std140UniformBlockRecordValue<BlockType> value = { buffer, bindingPoint };
            it->second->SetValue(&value);
        }
        else
        {
            auto rc = arena_.Create<Std140UniformBlockRecord<BlockType>>(block, buffer, bindingPoint);
            assignments_[key] = rc;
        }
    }

    /**
     * @brief 清空所有Variable-Value对
     */
    void Clear()
    {
        arena_.Clear();
        assignments_.clear();
    }

    /**
     * @brief 绑定所有Variable-Value对
     */
    void Bind() const noexcept
    {
        for(auto &it : assignments_)
            it.second->Bind();
    }
};

} // namespace AGZ::GraphicsAPI::GL
