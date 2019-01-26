#pragma once

#include <utility>

#include "AttribVariable.h"
#include "Buffer.h"

namespace AGZ::GraphicsAPI::GL
{
    
/**
 * @brief 对OpenGL中VAO的直接封装
 */
class VertexArray : public GLObject
{
public:

    /**
     * @param initHandle 是否立即创建一个GL Buffer Name
     */
    explicit VertexArray(bool initHandle = true) noexcept
        : GLObject(0)
    {
        if(initHandle)
            InitializeHandle();
    }

    VertexArray(VertexArray &&moveFrom) noexcept
        : GLObject(moveFrom.handle_)
    {
        moveFrom.handle_ = 0;
    }

    VertexArray &operator=(VertexArray &&moveFrom) noexcept
    {
        Destroy();
        std::swap(handle_, moveFrom.handle_);
        return *this;
    }

    ~VertexArray()
    {
        Destroy();
    }

    /**
     * @brief 创建一个VAO
     * @note 若已创建一个VAO且未经删除，则调用该函数的结果是未定义的
     */
    void InitializeHandle() noexcept
    {
        AGZ_ASSERT(!handle_);
        glCreateVertexArrays(1, &handle_);
    }

    /**
     * @brief 若含有VAO，将其标记为删除
     */
    void Destroy() noexcept
    {
        if(handle_)
        {
            glDeleteVertexArrays(1, &handle_);
            handle_ = 0;
        }
    }

    /**
     * @brief 取得VAO Name
     */
    GLuint GetHandle() const noexcept
    {
        return handle_;
    }

    /**
     * @brief 绑定指定的属性和binding point
     */
    template<typename VarType>
    void SetAttribBindingPoint(AttribVariable<VarType> var, GLuint bindingPoint) const noexcept
    {
        AGZ_ASSERT(handle_);
        glVertexArrayAttribBinding(handle_, var.GetLocation(), bindingPoint);
    }

    /**
     * @brief 设置某属性如何从VertexBuffer中读取数据
     */
    template<typename VarType>
    void SetAttribFormat(AttribVariable<VarType> var, GLuint byteRelativeOffset, bool normalized = false) const noexcept
    {
        AGZ_ASSERT(handle_);
        glVertexArrayAttribFormat(
            handle_, var.GetLocation(),
            Impl::Var2GL<VarType>::USize, Impl::Var2GL<VarType>::UType,
            normalized, byteRelativeOffset);
    }

    /**
     * @brief 绑定指定的VertexBuffer和binding point
     */
    template<typename VtxType>
    void SetVertexBufferBindingPoint(const VertexBuffer<VtxType> &buf, GLuint bindingPoint, size_t vertexOffset = 0) const noexcept
    {
        AGZ_ASSERT(handle_);
        glVertexArrayVertexBuffer(
            handle_, bindingPoint, buf.GetHandle(),
            static_cast<GLintptr>(sizeof(VtxType) * vertexOffset), GLsizei(sizeof(VtxType)));
    }

    /**
     * @brief 启用某属性从VertexBuffer中读取数据的功能
     */
    template<typename VarType>
    void EnableAttrib(AttribVariable<VarType> var) const noexcept
    {
        AGZ_ASSERT(handle_);
        glEnableVertexArrayAttrib(handle_, var.GetLocation());
    }

    /**
     * @brief 绑定指定属性和某VertexBuffer，SetAttribBindingPoint、SetAttribFormat、SetVertexBufferBindingPoint三合一
     */
    template<typename VtxType>
    void BindVertexBufferToAttrib(
        AttribVariable<VtxType> var, const VertexBuffer<VtxType> &buf,
        GLuint bindingPoint, bool normalized = false, size_t vertexOffset = 0) const noexcept
    {
        SetAttribBindingPoint(var, bindingPoint);
        SetAttribFormat(var, normalized, 0);
        SetVertexBufferBindingPoint(buf, bindingPoint, vertexOffset);
    }

    /**
     * @brief 绑定指定属性和某VertexBuffer中Vertex的指定成员，SetAttribBindingPoint、SetAttribFormat、SetVertexBufferBindingPoint三合一
     */
    template<typename VtxType, typename VarType>
    void BindVertexBufferToAttrib(
        AttribVariable<VarType> var, const VertexBuffer<VtxType> &buf, VarType (VtxType::*memPtr),
        GLuint bindingPoint, bool normalized = false, size_t vertexOffset = 0) const noexcept
    {
        SetAttribBindingPoint(var, bindingPoint);
        SetAttribFormat(var, GLuint(ByteOffsetOf(memPtr)), normalized);
        SetVertexBufferBindingPoint(buf, bindingPoint, vertexOffset);
    }

    /**
     * @brief 绑定该VAO中的Element Array Buffer
     */
    template<typename ElemType>
    void BindElementBuffer(ElementBuffer<ElemType> &buf) const noexcept
    {
        AGZ_ASSERT(handle_ && buf.GetHandle());
        glVertexArrayElementBuffer(handle_, buf.GetHandle());
    }

    /**
     * @brief 解除该VAO绑定的Element Array Buffer
     */
    void UnbindElementBuffer() const noexcept
    {
        AGZ_ASSERT(handle_);
        glVertexArrayElementBuffer(handle_, 0);
    }

    /**
     * @brief 将该VAO绑定到渲染管线
     */
    void Bind() const noexcept
    {
        AGZ_ASSERT(handle_);
        glBindVertexArray(handle_);
    }

    /**
     * @brief 将该VAO从渲染管线解绑
     */
    void Unbind() const noexcept
    {
        AGZ_ASSERT(handle_);
#       if defined(_DEBUG) || defined(DEBUG)
        GLint cur;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &cur);
        AGZ_ASSERT(cur == GLint(handle_));
#       endif
        glBindVertexArray(0);
    }
};

} // namespace AGZ::GraphicsAPI::GL
