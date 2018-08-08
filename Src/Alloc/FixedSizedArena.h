#pragma once

#include "../Misc/Common.h"
#include "Alloc.h"

AGZ_NS_BEG(AGZ)

template<typename Alloc = DefaultAllocator>
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
    Chunk *chunkEntry_;
    Node *freeNodes_;

public:

    FixedSizedArena(size_t nodeSize, size_t chunkSize)
        : nodeSize_(nodeSize), chunkSize_(chunkSize),
          freeNodes_(nullptr), chunkEntry_(nullptr)
    {
        if(nodeSize < sizeof(Node*) || nodeSize + sizeof(Chunk*) < chunkSize)
        {
            throw ArgumentException(
                "Invalid size arguments for FixedSizedArena");
        }
    }

    ~FixedSizedArena()
    {
        ReleaseAll();
    }

    FixedSizedArena(const FixedSizedArena<Alloc>&)            = delete;
    FixedSizedArena &operator=(const FixedSizedArena<Alloc>&) = delete;

    void *Malloc()
    {
        if(freeNodes_)
        {
            Node *ret = freeNodes_;
            freeNodes_ = freeNodes_->next;
            return ret;
        }

        Chunk *nChunk = Alloc::Malloc(chunkSize_);
        nChunk->next = chunkEntry_;
        chunkEntry_ = nChunk;

        char *node = nChunk->data;
        char *end = reinterpret_cast<char*>(nChunk) + chunkSize_ - nodeSize_;
        while(node <= end)
        {
            Free(node);
            node += nodeSize_;
        }

        return Malloc();
    }

    void Free(void *ptr)
    {
        Node *n = reinterpret_cast<Node*>(ptr);
        n->next = freeNodes_;
        freeNodes_ = n;
    }

    void ReleaseAll()
    {
        while(chunkEntry_)
        {
            Chunk *next = chunkEntry_->next;
            Alloc::Free(chunkEntry_);
            chunkEntry_ = next;
        }
    }
};

AGZ_NS_END(AGZ)
