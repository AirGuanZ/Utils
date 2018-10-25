#pragma once

#include "../Misc/Common.h"
#include "../Misc/Exception.h"
#include "../Misc/Uncopiable.h"
#include "Alloc.h"

AGZ_NS_BEG(AGZ)

/*
    С�����������������乹���������͵�С���󣬲�֧��ͳһ�ͷ�
    �ռ��þ����chunk list�����ѷ��������һ��freelist����

    chunk���֣�
        chunkEntry -> chunkWithFreeSpace -> chunkUsed -> chunkUsed -> ... -> nullptr

    node���֣�
        nodeEntry -> {
            nextNodePtr
            destructor
            obj
        }
*/
template<typename Alloc = CRTAllocator>
class ObjArena : public Uncopiable
{
    class DestructorInterface
    {
    public:

        DestructorInterface() noexcept = default;

        virtual ~DestructorInterface() = default;

        virtual size_t NodeSize() const = 0;

        virtual void Destruct() = 0;
    };

    template<typename T>
    class Destructor : public DestructorInterface
    {
    public:

        Destructor() noexcept = default;

        size_t NodeSize() const override
        {
            return sizeof(T) + sizeof(NodeHead) + sizeof(Destructor<T>);
        }

        void Destruct() override
        {
            T *dst = reinterpret_cast<T*>((
                reinterpret_cast<char*>(this) + sizeof(Destructor<T>)));
            dst->~T();
        }
    };

    struct ChunkHead
    {
        ChunkHead *nextChunk;
        char data[1];
    };

    struct NodeHead
    {
        NodeHead *nextNode;
    };

    ChunkHead *chunkEntry_;
    char *curChunkTop_;    // ��ǰchunk���������׸�δռ���ֽڵ�ַ
    size_t curChunkRest_;   // ��ǰchunk��ʣ�����ֽ�

    NodeHead *nodeEntry_;

    const size_t chunkDataSize_;
    size_t usedBytes_;

    template<typename T>
    static size_t ObjSize2NodeSize()
    {
        return sizeof(T) + sizeof(NodeHead) + sizeof(Destructor<T>);
    }

    void AllocNewChunk()
    {
        size_t size = sizeof(ChunkHead) + chunkDataSize_ - 1;
        char *data = static_cast<char*>(Alloc::Malloc(size));

        usedBytes_ += curChunkRest_ + size - chunkDataSize_;

        ChunkHead *head = reinterpret_cast<ChunkHead*>(data);
        head->nextChunk = chunkEntry_;
        chunkEntry_ = head;
        curChunkTop_ = head->data;
        curChunkRest_ = chunkDataSize_;
    }

public:

    explicit ObjArena(size_t chunkDataSize = 1025 - sizeof(ChunkHead))
        : chunkEntry_(nullptr), curChunkTop_(nullptr), curChunkRest_(0),
          nodeEntry_(nullptr), chunkDataSize_(chunkDataSize), usedBytes_(0)
    {
        if(chunkDataSize < 1)
            throw ArgumentException("ObjArena: chunkDataSize must be positive");
    }

    ~ObjArena()
    {
        Clear();
    }

    size_t GetUsedBytes() const
    {
        return usedBytes_;
    }

    template<typename T, typename...Args>
    T *Create(Args&&...args)
    {
        size_t nodeSize = ObjSize2NodeSize<T>();

        // ����Ķ���ֱ����Alloc::Malloc����
        if(nodeSize > chunkDataSize_)
        {
            char *data = static_cast<char*>(Alloc::Malloc(nodeSize));
            char *destructor = data + sizeof(NodeHead);
            char *obj = destructor + sizeof(Destructor<T>);

            try
            {
                new(obj) T(std::forward<Args>(args)...);
            }
            catch(...)
            {
                Alloc::Free(data);
                throw;
            }

            new(destructor) Destructor<T>();

            NodeHead *newNode = reinterpret_cast<NodeHead*>(data);
            newNode->nextNode = nodeEntry_;
            nodeEntry_ = newNode;
            usedBytes_ += nodeSize;

            return reinterpret_cast<T*>(obj);
        }

        // �����ǰchunkʣ��ռ䲻�㣬�ͷ���һ����chunk
        if(curChunkRest_ < nodeSize)
            AllocNewChunk();
        AGZ_ASSERT(curChunkRest_ >= nodeSize);

        char *data = curChunkTop_;
        char *destructor = data + sizeof(NodeHead);
        char *obj = destructor + sizeof(Destructor<T>);

        new(obj) T(std::forward<Args>(args)...);
        new(destructor) Destructor<T>();

        NodeHead *newNode = reinterpret_cast<NodeHead*>(data);
        newNode->nextNode = nodeEntry_;
        nodeEntry_ = newNode;

        curChunkTop_ += nodeSize;
        curChunkRest_ -= nodeSize;
        usedBytes_ += nodeSize;

        return reinterpret_cast<T*>(obj);
    }

    void Clear()
    {
        // ���������ѷ���Ķ���
        for(NodeHead *nodeHead = nodeEntry_, *next; nodeHead; nodeHead = next)
        {
            next = nodeHead->nextNode;

            auto destructor = reinterpret_cast<DestructorInterface*>(
                reinterpret_cast<char*>(nodeHead) + sizeof(NodeHead));
            destructor->Destruct();
            destructor->~DestructorInterface();

            if(destructor->NodeSize() > chunkDataSize_)
                Alloc::Free(nodeHead);
        }

        // �ͷ�����chunk
        for(ChunkHead *chunkHead = chunkEntry_, *next; chunkHead; chunkHead = next)
        {
            next = chunkHead->nextChunk;
            Alloc::Free(chunkHead);
        }

        chunkEntry_ = nullptr;
        curChunkTop_ = nullptr;
        curChunkRest_ = 0;

        nodeEntry_ = nullptr;

        usedBytes_ = 0;
    }
};

AGZ_NS_END(AGZ)
