#pragma once

/**
 * @file Alloc/FixedSizedArena.h
 * @brief 定义了用于分配固定大小内存快的内存池
 */

#include "../Misc/Exception.h"
#include "Alloc.h"

namespace AGZ {

/**
 * @brief 用于分配固定大小块的内存池
 * 
 * 内部为经典的Chunk-Node两级结构，Chunk以侵入式链表连接以供统一释放，
 * 空闲Node以freelist连接
 */
template<typename BaseAlloc = DefaultAllocator>
class FixedSizedArena
{
    struct Chunk
    {
        Chunk *next;
        char data[1];
    };

    struct Node
    {
        Node *next;
    };

    size_t nodeSize_;
    size_t chunkSize_;

    Node *freeNodes_;
    Chunk *chunkEntry_;

    void FreeAllImpl()
    {
        while(chunkEntry_)
        {
            Chunk *next = chunkEntry_->next;
            BaseAlloc::Free(chunkEntry_);
            chunkEntry_ = next;
        }
    }

public:

    /**
     * @brief 指定以后每次分配的内存块大小
     * 
     * 默认每个Chunk包含32个node
     *
     * @param nodeSize 以后可分配的内存块大小
     */
    explicit FixedSizedArena(size_t nodeSize)
        : FixedSizedArena(nodeSize, 32)
    {

    }

    /**
     * @brief 指定以后每次分配的内存块大小，以及预分配的粒度
     * 
     * @param nodeSize 每次分配的内存块字节数
     * @param chunkNodeCount 每个Chunk包含多少个node
     * 
     * @exception ArgumentException 参数非法时抛出
     */
    FixedSizedArena(size_t nodeSize, size_t chunkNodeCount)
        : nodeSize_(nodeSize), chunkSize_(nodeSize * chunkNodeCount + sizeof(Chunk*)),
          freeNodes_(nullptr), chunkEntry_(nullptr)
    {
        if(nodeSize < sizeof(Node*) || !chunkNodeCount)
        {
            throw ArgumentException(
                "Invalid size arguments for FixedSizedArena");
        }
    }

    ~FixedSizedArena()
    {
        FreeAllImpl();
    }

    /**
     * @brief 快速取得一块固定大小的内存
     * 
     * 大部分情况下会从预分配的空间中取出内存块，当预分配空间不足时会用BaseAlloc扩大预分配空间
     * 
     * @exception std::bad_alloc 预分配空间不足且扩充失败时抛出
     */
    void *Alloc()
    {
        if(freeNodes_)
        {
            Node *ret = freeNodes_;
            freeNodes_ = freeNodes_->next;
            return ret;
        }

        auto *nChunk = reinterpret_cast<Chunk*>(BaseAlloc::Malloc(chunkSize_));
        nChunk->next = chunkEntry_;
        chunkEntry_ = nChunk;

        char *node = nChunk->data;
        char *end = reinterpret_cast<char*>(nChunk) + chunkSize_ - nodeSize_;
        while(node <= end)
        {
            Free(node);
            node += nodeSize_;
        }

        return Alloc();
    }

    /**
     * @brief 释放一块由Alloc返回的内存
     * 
     * 这块内存不会被返还给操作系统，而是在以后调用Alloc时优先使用
     * 
     * @param ptr 待释放内存块的首字节地址
     */
    void Free(void *ptr) noexcept
    {
        auto *n = reinterpret_cast<Node*>(ptr);
        n->next = freeNodes_;
        freeNodes_ = n;
    }

    /**
     * @brief 释放所有预分配空间
     * 
     * @warning 这一操作会使得所有由Alloc返回的内存块失效
     */
    void FreeAll()
    {
        FreeAllImpl();
    }
};

} // namespace AGZ
