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

/**
 * @brief 交换任意维张量的维度
 * @param pData 指向张量首元素的指针
 * @param shape 原张量大小，pData布局应和数组T[shape[0]][shape[1]]...[shape[N-1]]相同
 * @param perm 交换后的维度排布，如{ 2, 0, 1 }就是把原来的{ 0, 1, 2 }换成{ 2, 0, 1 }
 * @param pNewShape 用于获取新张量的大小，默认为空指针
 */
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
