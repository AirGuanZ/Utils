#pragma once

#include <tuple>
#include <unordered_map>

#include <AGZUtils/Utils/Alloc.h>

#include "Common.h"

namespace AGZ::GL
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
    class Record : public RecordInterface
    {
    public:

        UniformVariable<VarType> var;
        VarType value;

        Record(UniformVariable<VarType> var, const VarType &value) noexcept
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
            auto *rc = arena_.Create<Record<VarType>>(var, value);
            assignments_[var.GetLocation()] = rc;
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
    void Bind() noexcept
    {
        for(auto &it : assignments_)
            it.second->Bind();
    }
};

} // namespace AGZ::GL
