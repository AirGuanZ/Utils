#pragma once

#include "../Utils/Math.h"

namespace AGZ {

/**
 * @brief 立方纹理映射
 * 
 * 并不支持对立方体纹理坐标进行各种旋转、翻转的倒腾，自己试吧……
 */
template<typename T>
class CubeMapper
{
public:

    /**
     * 面编号，表示方向被映射至哪个面所对应的纹理
     */
    enum FaceIndex
    {
        POS_X = 0,
        POS_Y = 1,
        POS_Z = 2,
        NEG_X = 3,
        NEG_Y = 4,
        NEG_Z = 5,
    };

    /**
     * 映射结果
     */
    struct MapResult
    {
        FaceIndex face;   // 给定方向被映射至哪个面
        Math::Vec2<T> uv; // 这个面上的uv值
    };

    /**
     * 将方向向量映射至立方纹理坐标
     */
    static MapResult Map(const Math::Vec3<T> &dir);
};

namespace Impl
{
    template<typename T, int Axis, bool Neg>
    Option<Math::Vec3<T>> AlignedQuadInct(const Math::Vec3<T> &dir)
    {
        if(!dir[Axis])
            return None;

        T t;
        if constexpr(!Neg)
            t = T(1) / dir[Axis];
        else
            t = T(-1) / dir[Axis];

        auto p = t * dir;
        if(t < 0 ||
           Math::Abs(p[(Axis + 1) % 3]) > T(1) ||
           Math::Abs(p[(Axis + 2) % 3]) > T(1))
            return None;

        return p;
    }
}

template <typename T>
typename CubeMapper<T>::MapResult CubeMapper<T>::Map(const Math::Vec3<T> &dir)
{
    using Math::Clamp;

    int bestAxis; bool neg;
    Option<Math::Vec3<T>> op;

    constexpr int X = 0, Y = 1, Z = 2;

    if((op = Impl::AlignedQuadInct<T, X, false>(dir)))
        bestAxis = X, neg = false;
    else if((op = Impl::AlignedQuadInct<T, Y, false>(dir)))
        bestAxis = Y, neg = false;
    else if((op = Impl::AlignedQuadInct<T, Z, false>(dir)))
        bestAxis = Z, neg = false;
    else if((op = Impl::AlignedQuadInct<T, X, true>(dir)))
        bestAxis = X, neg = true;
    else if((op = Impl::AlignedQuadInct<T, Y, true>(dir)))
        bestAxis = Y, neg = true;
    else if((op = Impl::AlignedQuadInct<T, Z, true>(dir)))
        bestAxis = Z, neg = true;
    else
        Unreachable();

    switch(bestAxis)
    {
    case X:
        if(neg)
        {
            T u = Clamp(T(0.5) - T(0.5) * op->y, T(0), T(1));
            T v = Clamp(T(0.5) * op->z + T(0.5), T(0), T(1));
            return { NEG_X, { u, v } };
        }
        else
        {
            T u = Clamp(T(0.5) * op->y + T(0.5), T(0), T(1));
            T v = Clamp(T(0.5) * op->z + T(0.5), T(0), T(1));
            return { POS_X, { u, v } };
        }
    case Y:
        if(neg)
        {
            T u = Clamp(T(0.5) * op->x + T(0.5), T(0), T(1));
            T v = Clamp(T(0.5) * op->z + T(0.5), T(0), T(1));
            return { NEG_Y, { u, v } };
        }
        else
        {
            T u = Clamp(T(0.5) - T(0.5) * op->x, T(0), T(1));
            T v = Clamp(T(0.5) * op->z + T(0.5), T(0), T(1));
            return { POS_Y, { u, v } };
        }
    case Z:
        if(neg)
        {
            T u = Clamp(T(0.5) - T(0.5) * op->x, T(0), T(1));
            T v = Clamp(T(0.5) - T(0.5) * op->y, T(0), T(1));
            return { NEG_Z, { 1 - u, v } };
        }
        else
        {
            T u = Clamp(T(0.5) * op->x + T(0.5), T(0), T(1));
            T v = Clamp(T(0.5) * op->y + T(0.5), T(0), T(1));
            return { POS_Z, { u, v } };
        }
    default:
        Unreachable();
    }
}

} // namespace AGZ
