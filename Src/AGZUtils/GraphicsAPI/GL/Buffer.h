#pragma once

#include <AGZUtils/Utils/Misc.h>

#include "Common.h"

namespace AGZ::GL
{

/**
 * @brief 对OpenGL Buffer Object的直接封装
 */
class Buffer : public GLObject
{
protected:

    /**
     * @param initHandle 是否立即创建一个GL Buffer Name
     */
    explicit Buffer(bool initHandle = false) noexcept
    {
        if(initHandle)
            InitializeHandle();
    }

    Buffer(Buffer &&moveFrom) noexcept
        : GLObject(moveFrom.handle_)
    {
        moveFrom.handle_ = 0;
    }

    Buffer &operator=(Buffer &&moveFrom) noexcept
    {
        Destroy();
        std::swap(handle_, moveFrom.handle_);
        return *this;
    }

    ~Buffer()
    {
        Destroy();
    }

    /**
     * @brief 创建一个GL Buffer Name
     * @note 若已创建一个Name且未经删除，则调用该函数的结果是未定义的
     */
    void InitializeHandle() noexcept
    {
        AGZ_ASSERT(!handle_);
        glCreateBuffers(1, &handle_);
    }

    /**
     * @brief 若含有Buffer Object，将该Buffer标记为删除
     */
    void Destroy() noexcept
    {
        if(handle_)
        {
            glDeleteBuffers(1, &handle_);
            handle_ = 0;
        }
    }

    /**
     * @brief 初始化该Buffer内部的数据。若已有数据，则释放并重新申请存储空间并初始化。
     * @param data 初始化数据指针
     * @param byteSize 初始化数据的字节数
     * @param usage 如GL_STATIC_DRAW
     */
    void ReinitializeData(const void *data, size_t byteSize, GLenum usage) const noexcept
    {
        AGZ_ASSERT(handle_);
        glNamedBufferData(handle_, static_cast<GLsizei>(byteSize), data, usage);
    }

    /**
     * @brief 设置Buffer的部分内容
     * @param subdata 待写入buffer的数据
     * @param byteOffset 要设置的内容据buffer开头的距离
     * @param byteSize 要设置的内容的长度
     */
    void SetData(const void *subdata, size_t byteOffset, size_t byteSize) const noexcept
    {
        AGZ_ASSERT(handle_);
        glNamedBufferSubData(handle_, static_cast<GLsizei>(byteOffset), static_cast<GLsizei>(byteSize), subdata);
    }
};

/**
 * @brief 对Vertex Buffer的直接封装。和原生Buffer相比，主要特点是带有类型信息。
 * @tparam VarType 顶点数据类型
 */
template<typename VarType>
class VertexBuffer : public Buffer
{
    size_t vertexCount_;

public:

    /**
     * @param initHandle 是否立即创建一个GL Buffer Name
     */
    explicit VertexBuffer(bool initHandle = false) noexcept
        : Buffer(initHandle), vertexCount_(0)
    {
        
    }

    /**
     * @brief 立刻创建一个Buffer Name并用给定的数据初始化其内容
     * @param data 用于初始化的数据指针
     * @param vertexCount 有多少个顶点数据
     * @param usage 如GL_STATIC_DRAW
     */
    VertexBuffer(const VarType *data, size_t vertexCount, GLenum usage) noexcept
        : VertexBuffer(true)
    {
        ReinitializeData(data, vertexCount, usage);
    }

    VertexBuffer(VertexBuffer<VarType> &&moveFrom) noexcept
        : Buffer(std::move(static_cast<Buffer&>(moveFrom))), vertexCount_(moveFrom.vertexCount_)
    {
        moveFrom.vertexCount_ = 0;
    }

    VertexBuffer<VarType> &operator=(VertexBuffer<VarType> &&moveFrom) noexcept
    {
        static_cast<Buffer&>(*this) = std::move(static_cast<Buffer&>(moveFrom));
        std::swap(vertexCount_, moveFrom.vertexCount_);
        return *this;
    }

    ~VertexBuffer() = default;

    /**
     * @brief 创建一个GL Buffer Name
     * @note 若已创建一个Name且未经删除，则调用该函数的结果是未定义的
     */
    void InitializeHandle() noexcept
    {
        Buffer::InitializeHandle();
    }

    /**
     * @brief 若含有Buffer Object，将该Buffer标记为删除
     */
    void Destroy() noexcept
    {
        Buffer::Destroy();
        vertexCount_ = 0;
    }

    /**
     * @brief 初始化该Buffer内部的数据。若已有数据，则释放并重新申请存储空间并初始化。
     * @param data 初始化数据指针
     * @param vertexCount 初始化数据所包含的顶点数
     * @param usage 如GL_STATIC_DRAW
     */
    void ReinitializeData(const VarType *data, size_t vertexCount, GLenum usage) noexcept
    {
        AGZ_ASSERT(vertexCount);
        Buffer::ReinitializeData(data, sizeof(VarType) * vertexCount, usage);
        vertexCount_ = vertexCount;
    }

    /**
     * @brief 设置Buffer的部分内容
     * @param data 待写入buffer的数据
     * @param vertexOffset 要设置的内容据buffer开头有多少个顶点
     * @param vertexCount 要设置的内容包含多少个顶点
     */
    void SetData(const VarType *data, size_t vertexOffset, size_t vertexCount) const noexcept
    {
        AGZ_ASSERT(vertexCount && vertexOffset + vertexCount <= vertexCount_);
        Buffer::SetData(data, vertexOffset * sizeof(VarType), vertexCount * sizeof(VarType));
    }

    /**
     * @brief 取得内部所存储的顶点数量
     */
    size_t GetVertexCount() const noexcept
    {
        return vertexCount_;
    }
};

} // namespace AGZ::GL
