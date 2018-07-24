#pragma once

#include "../Common.h"

AGZ_NS_BEG(AGZ)

namespace Aux
{
    template<typename T>
    constexpr T StaticMax(T a, T b) { return a > b ? a : b; }
}

template<typename T, typename F>
class alignas(alignof(T), alignof(F)) FixedResult
{
    unsigned char data_[Aux::StaticMax<size_t>(sizeof(T), sizeof(F))];
    bool isSome_;
    
public:

    enum Type { Ok, Err };
    using OkData  = T;
    using ErrData = F;
    using Self    = FixedResult<T, F>;
    
    
};

AGZ_NS_END(AGZ)
