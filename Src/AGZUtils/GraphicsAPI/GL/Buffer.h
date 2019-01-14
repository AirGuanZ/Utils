#pragma once

#include <type_traits>

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
    uint32_t vertexCount_;

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
    VertexBuffer(const VarType *data, uint32_t vertexCount, GLenum usage) noexcept
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
        vertexCount_ = moveFrom.vertexCount_;
        moveFrom.vertexCount_ = 0;
        return *this;
    }

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
    void ReinitializeData(const VarType *data, uint32_t vertexCount, GLenum usage) noexcept
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
    void SetData(const VarType *data, uint32_t vertexOffset, uint32_t vertexCount) const noexcept
    {
        AGZ_ASSERT(vertexCount && vertexOffset + vertexCount <= vertexCount_);
        Buffer::SetData(data, vertexOffset * sizeof(VarType), vertexCount * sizeof(VarType));
    }

    /**
     * @brief 取得内部所存储的顶点数量
     */
    uint32_t GetVertexCount() const noexcept
    {
        return vertexCount_;
    }
};

/**
 * @brief 对适用于std140 layout的Uniform Block Object的直接封装
 */
template<typename BlockType>
class Std140UniformBlockBuffer : public Buffer
{
public:

    /**
     * @param initHandle 是否立即创建一个GL Buffer Name
     */
    explicit Std140UniformBlockBuffer(bool initHandle = false) noexcept
        : Buffer(initHandle)
    {

    }

    /**
     * @brief 立刻创建一个Buffer Name并用给定的数据初始化其内容
     * @param data 用于初始化的数据指针
     * @param usage 如GL_STATIC_DRAW
     */
    Std140UniformBlockBuffer(const BlockType *data, GLenum usage) noexcept
        : Std140UniformBlockBuffer(true)
    {
        ReinitializeData(data, usage);
    }

    Std140UniformBlockBuffer(Std140UniformBlockBuffer<BlockType> &&moveFrom) noexcept
        : Buffer(std::move(static_cast<Buffer&>(moveFrom)))
    {

    }

    Std140UniformBlockBuffer<BlockType> &operator=(Std140UniformBlockBuffer<BlockType> &&moveFrom) noexcept
    {
        static_cast<Buffer&>(*this) = std::move(static_cast<Buffer&>(moveFrom));
        return *this;
    }

    ~Std140UniformBlockBuffer() = default;

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
    }

    /**
     * @brief 初始化该Buffer内部的数据。若已有数据，则释放并重新申请存储空间并初始化。
     * @param data 初始化数据指针
     * @param usage 如GL_STATIC_DRAW
     */
    void ReinitializeData(const BlockType *data, GLenum usage) const noexcept
    {
        AGZ_ASSERT(handle_);
        glNamedBufferData(handle_, sizeof(BlockType), data, usage);
    }

    /**
     * @brief 设置Buffer的部分内容
     * @param subdata 待写入buffer的数据
     * @param byteOffset 要设置的内容距buffer开头的距离
     * @param byteSize 要设置的内容的长度
     */
    void SetData(const void *subdata, size_t byteOffset, size_t byteSize) const noexcept
    {
        AGZ_ASSERT(handle_);
        glNamedBufferSubData(handle_, static_cast<GLsizei>(byteOffset), static_cast<GLsizei>(byteSize), subdata);
    }

    /**
     * @brief 设置整个Buffer的内容
     * @param data 待写入buffer的数据
     */
    void SetData(const BlockType *data) const noexcept
    {
        AGZ_ASSERT(handle_);
        glNamedBufferSubData(handle_, 0, sizeof(BlockType), data);
    }

    /**
     * @brief 将该UBO绑定到指定的binding point
     */
    void Bind(GLuint bindingPoint) const noexcept
    {
        AGZ_ASSERT(handle_);
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, handle_);
    }
};

/**
 * @brief 对OpenGL Element Buffer的直接封装
 */
template<typename ElemType_>
class ElementBuffer : public Buffer
{
    static_assert(std::is_same_v<ElemType_, GLubyte> || std::is_same_v<ElemType_, GLushort> || std::is_same_v<ElemType_, GLuint>,
        "IndexType of ElementBuffer must be one of { GLubyte, GLushort, GLuint }");

    uint32_t elemCount_;

public:

    using ElemType = ElemType_;

    /**
     * @param initHandle 是否立即创建一个GL Buffer Name
     */
    explicit ElementBuffer(bool initHandle = false) noexcept
        : Buffer(initHandle), elemCount_(0)
    {
        
    }

    /**
     * @brief 立刻创建一个Buffer Name并用给定的数据初始化其内容
     * @param data 用于初始化的数据指针
     * @param elemCount element数量
     * @param usage 如GL_STATIC_DRAW
     */
    ElementBuffer(const ElemType *data, uint32_t elemCount, GLenum usage) noexcept
        : ElementBuffer(true)
    {
        ReinitializeData(data, elemCount, usage);
    }

    ElementBuffer(ElementBuffer<ElemType> &&moveFrom) noexcept
        : Buffer(std::move(static_cast<Buffer&>(moveFrom))), elemCount_(moveFrom.elemCount_)
    {
        moveFrom.elemCount_ = 0;
    }

    ElementBuffer<ElemType> &operator=(ElementBuffer<ElemType> &&moveFrom) noexcept
    {
        static_cast<Buffer&>(*this) = std::move(static_cast<Buffer&>(moveFrom));
        elemCount_ = moveFrom.elemCount_;
        moveFrom.elemCount_ = 0;
        return *this;
    }

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
        elemCount_ = 0;
    }

    /**
     * @brief 初始化该Buffer内部的数据。若已有数据，则释放并重新申请存储空间并初始化。
     * @param data 初始化数据指针
     * @param elemCount element数量
     * @param usage 如GL_STATIC_DRAW
     */
    void ReinitializeData(const ElemType *data, uint32_t elemCount, GLenum usage) noexcept
    {
        AGZ_ASSERT(elemCount);
        Buffer::ReinitializeData(data, sizeof(ElemType) * elemCount, usage);
        elemCount_ = elemCount;
    }

    /**
     * @brief 设置Buffer的部分内容
     * @param data 待写入buffer的数据
     * @param elemOffset 要设置的内容距buffer开头有多少个element
     * @param elemCount 要设置的element的数量
     */
    void SetData(const ElemType *data, uint32_t elemOffset, uint32_t elemCount) const noexcept
    {
        AGZ_ASSERT(elemCount && elemOffset + elemCount <= elemCount_);
        Buffer::SetData(data, elemOffset * sizeof(ElemType), elemCount * sizeof(ElemType));
    }

    /**
     * @brief 取得element数量
     */
    uint32_t GetElemCount() const noexcept
    {
        return elemCount_;
    }

    /**
     * @brief 取得element对应的GL类型
     */
    constexpr GLenum GetElemType() const noexcept
    {
        if constexpr(std::is_same_v<ElemType, GLubyte>)
            return GL_UNSIGNED_BYTE;
        else if constexpr(std::is_same_v<ElemType, GLushort>)
            return GL_UNSIGNED_SHORT;
        else
            return GL_UNSIGNED_INT;
    }
};

} // namespace AGZ::GL
