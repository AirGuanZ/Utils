#pragma once

#include "../Common.h"
#include "Default.h"

AGZ_NS_BEG(AGZ::Alloc)

class FastAllocator
{
    struct Node
    {
        Node *next;
        void *top;
        size_t remainSize;
    };

    Node *cur_;
    size_t nodeSize_;

    void NewNode(size_t size)
    {
        Node *n = (Node*)std::malloc(size + sizeof(Node));
        n->next = cur_;
        n->top = (char*)n + sizeof(Node);
        n->remainSize = size;
        cur_ = n;
    }

public:
    FastAllocator(size_t nodeSize = 1024)
        : cur_(nullptr), nodeSize_(nodeSize)
    {
        AGZ_ASSERT(nodeSize_ > 0);
    }

    ~FastAllocator() { FreeAll(); }

    void *Allocate(size_t size)
    {
        AGZ_ASSERT(size > 0);

        if(size > nodeSize_)
        {
            NewNode(size);
            cur->remainSize = 0;
            return cur->top;
        }

        if(!cur_ || size > cur_->remainSize)
            NewNode(nodeSize_);
        void *ret = cur_->top;
        cur_->top = (char*)cur_->top + size;
        cur_->remainSize -= size;
        return ret;
    }

    void Deallocate(void *ptr) { }

    void FreeAll()
    {
        while(cur_)
        {
            Node *next = cur_->next;
            std::free(cur_);
            cur_ = next;
        }
    }
};

AGZ_NS_END(AGZ::Alloc)
