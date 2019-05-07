#pragma once

#include <vector>

#include "Vec.h"

namespace AGZ::Math
{

namespace PermuteImpl
{
    template<int N>
    int ToLinearIndex(const Vec<N, int> &shape, const Vec<N, int> &idx)
    {
        int ret = 0, base = 1;
        for(int i = N - 1; i >= 0; --i)
        {
            ret += idx[i] * base;
            base *= shape[i];
        }
        return ret;
    }

    template<int N>
    Vec<N, int> PermuteIndex(const Vec<N, int> &idx, const Vec<N, int> &perm)
    {
        Vec<N, int> ret;
        for(int i = 0; i < N; ++i)
            ret[i] = idx[perm[i]];
        return ret;
    }

    template<int N>
    Vec<N, int> NextIndex(const Vec<N, int> &shape, const Vec<N, int> &idx)
    {
        auto ret = idx;
        for(int i = N - 1; i >= 1; --i)
        {
            if(++ret[i] < shape[i])
                return ret;
            ret[i] = 0;
        }
        ++ret[0];
        return ret;
    }
}
    
template<int N, typename T>
void Permute(T *pData, const Vec<N, int> &shape, const Vec<N, int> &perm, Vec<N, int> *pNewShape = nullptr)
{
    using namespace PermuteImpl;

    if constexpr(N == 1)
        return;

    Vec<N, int> newShape;
    for(int i = 0; i < N; ++i)
        newShape[i] = shape[perm[i]];

    int dataCount = shape.Product();
    std::vector<T> tData(dataCount);

    Vec<N, int> idx;
    for(int i = 0; i < dataCount; ++i)
    {
        int srcLinearIdx = ToLinearIndex<N>(shape, idx);
        int dstLinearIdx = ToLinearIndex<N>(newShape, PermuteIndex<N>(idx, perm));
        tData[dstLinearIdx] = std::move(pData[srcLinearIdx]);
        idx = NextIndex<N>(shape, idx);
    }

    for(int i = 0; i < dataCount; ++i)
        pData[i] = std::move(tData[i]);

    if(pNewShape)
        *pNewShape = newShape;
}

} // namespace AGZ::Math
