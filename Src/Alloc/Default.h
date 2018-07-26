#pragma once

#include <cstdlib>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ::Alloc)

class DefaultAllocator
{
public:
    void *Allocate(size_t size) { return std::malloc(size); }
    void Deallocate(void *ptr)  { std::free(ptr); }
};

AGZ_NS_END(AGZ::Alloc)
