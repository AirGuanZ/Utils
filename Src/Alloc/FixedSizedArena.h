#pragma once

#include "../Misc/Common.h"
#include "../Misc/Exception.h"
#include "Alloc.h"
#include "Arena.h"

AGZ_NS_BEG(AGZ)

template<typename BaseAlloc = DefaultAllocator>
class FixedSizedArena : public ArbitaryArena
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

    explicit FixedSizedArena(size_t nodeSize)
        : FixedSizedArena(nodeSize, nodeSize * 32 + sizeof(Chunk*))
    {

    }

    FixedSizedArena(size_t nodeSize, size_t chunkByteSize)
        : nodeSize_(nodeSize), chunkSize_(chunkByteSize),
          freeNodes_(nullptr), chunkEntry_(nullptr)
    {
        if(nodeSize < sizeof(Node*) || nodeSize + sizeof(Chunk*) > chunkByteSize)
        {
            throw ArgumentException(
                "Invalid size arguments for FixedSizedArena");
        }
    }

    ~FixedSizedArena()
    {
        FreeAllImpl();
    }

    void *Alloc(size_t _size) override
    {
        if(freeNodes_)
        {
            Node *ret = freeNodes_;
            freeNodes_ = freeNodes_->next;
            return ret;
        }

        Chunk *nChunk = reinterpret_cast<Chunk*>(BaseAlloc::Malloc(chunkSize_));
        nChunk->next = chunkEntry_;
        chunkEntry_ = nChunk;

        char *node = nChunk->data;
        char *end = reinterpret_cast<char*>(nChunk) + chunkSize_ - nodeSize_;
        while(node <= end)
        {
            Free(node);
            node += nodeSize_;
        }

        return Alloc(0);
    }

    void Free(void *ptr) override
    {
        Node *n = reinterpret_cast<Node*>(ptr);
        n->next = freeNodes_;
        freeNodes_ = n;
    }

    void FreeAll() override
    {
        FreeAllImpl();
    }
};

template<typename E, typename BaseAlloc = DefaultAllocator>
class SmallObjArena : public FixedArena<E>
{
    FixedSizedArena<BaseAlloc> base_;

public:

    using Self = SmallObjArena<E, BaseAlloc>;

    explicit SmallObjArena(size_t chunkSize = 32 * sizeof(E))
        : base_(sizeof(E), chunkSize)
    {

    }

    SmallObjArena(const Self &)   = delete;
    Self &operator=(const Self &) = delete;
    ~SmallObjArena()              = default;

    E *Alloc() override
    {
        return static_cast<E*>(base_.Alloc(0));
    }

    void Free(E *ptr) override
    {
        base_.Free(ptr);
    }
};

AGZ_NS_END(AGZ)
