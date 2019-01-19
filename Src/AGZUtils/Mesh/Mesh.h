#pragma once

#include <algorithm>
#include <map>
#include <numeric>
#include <set>
#include <vector>

#include "../Misc/Common.h"
#include "../Utils/Math.h"
#include "../Utils/String.h"

namespace AGZ::Mesh {

/**
 * @brief AABB包围盒，用于表达物体的轴对称体积
 */
template<typename T>
struct BoundingBox
{
    Math::Vec3<T> low  = Math::Vec3<T>(+Math::Inf<T>);
    Math::Vec3<T> high = Math::Vec3<T>(-Math::Inf<T>);

    /**
     * @brief 将给定点纳入包围范围中
     */
    BoundingBox<T> &Expand(const Math::Vec3<T> &p) noexcept
    {
        low.x = Math::Min(low.x, p.x);
        low.y = Math::Min(low.y, p.y);
        low.z = Math::Min(low.z, p.z);
        high.x = Math::Max(high.x, p.x);
        high.y = Math::Max(high.y, p.y);
        high.z = Math::Max(high.z, p.z);
        return *this;
    }

    /**
     * @brief 将给定包围盒纳入包围范围中
     */
    BoundingBox<T> &Union(const BoundingBox<T> &b) noexcept
    {
        low.x = Math::Min(low.x, b.low.x);
        low.y = Math::Min(low.y, b.low.y);
        low.z = Math::Min(low.z, b.low.z);
        high.x = Math::Max(high.x, b.high.x);
        high.y = Math::Max(high.y, b.high.y);
        high.z = Math::Max(high.z, b.high.z);
        return *this;
    }
};

/**
 * @brief 简单的几何模型，仅包含顶点位置、纹理坐标以及法线
 */
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
struct GeometryMesh
{
    struct Vertex
    {
        Math::Vec3<T> pos;
        Math::Vec3<T> tex;
        Math::Vec3<T> nor;
    };

    std::vector<Vertex> vertices;

    /**
     * @brief 自动平滑模型法线
     * 
     * @warning 算法非常糙，慎用
     */
    GeometryMesh<T> &SmoothenNormals();

    /**
     * @brief 取得整个mesh的AABB包围盒
     */
    BoundingBox<T> GetBoundingBox() const noexcept;
};

/**
 * @brief 简单的几何模型组，为从名字到几何模型的映射
 */
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
struct GeometryMeshGroup
{
    std::map<Str8, GeometryMesh<T>> submeshes;

    /**
     * @brief 自动平滑模型法线
     *
     * @warning 算法非常糙，慎用
     */
    GeometryMeshGroup<T> &SmoothenNormals();

    /**
     * @brief 把submeshes合并成一个mesh并返回
     */
    GeometryMesh<T> MergeAllSubmeshes() const;

    /**
     * @brief 取得整个mesh group的AABB包围盒
     */
    BoundingBox<T> GetBoundingBox() const noexcept;
};

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> DN>
GeometryMesh<T> &GeometryMesh<T, DN>::SmoothenNormals()
{
    // IMPROVE

    struct CompVec3
    {
        bool operator()(const Math::Vec3<T> &lhs, const Math::Vec3<T> &rhs) const
        {
            return lhs.x < rhs.x ||
                  (lhs.x == rhs.x && lhs.y < rhs.y) ||
                  (lhs.xy() == rhs.xy() && lhs.z < rhs.z);
        }
    };

    std::map<Math::Vec3<T>, std::set<size_t>, CompVec3> vtx2Nors;
    for(size_t i = 0; i < vertices.size(); ++i)
        vtx2Nors[vertices[i].pos].insert(i);

    for(auto &p : vtx2Nors)
    {
        auto avgNor = std::accumulate(p.second.begin(), p.second.end(), Math::Vec3<T>(0.0),
            [=](const Math::Vec3<T> &L, size_t R) { return L + this->vertices[R].nor; });
        if(avgNor.LengthSquare() < T(0.001))
            continue;
        avgNor = avgNor.Normalize();
        for(auto i : p.second)
            vertices[i].nor = avgNor;
    }

    return *this;
}

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> N>
BoundingBox<T> GeometryMesh<T, N>::GetBoundingBox() const noexcept
{
    BoundingBox<T> ret;
    for(auto &v : vertices)
        ret.Expand(v.pos);
    return ret;
}

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> DN>
GeometryMeshGroup<T> &GeometryMeshGroup<T, DN>::SmoothenNormals()
{
    for(auto &p : submeshes)
        p.second.SmoothenNormals();
    return *this;
}

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> DN>
GeometryMesh<T> GeometryMeshGroup<T, DN>::MergeAllSubmeshes() const
{
    GeometryMesh<T> ret;
    for(auto &it : submeshes)
    {
        auto &sm = it.second;
        std::copy(std::begin(sm.vertices), std::end(sm.vertices),
                  std::back_inserter(ret.vertices));
    }
    return ret;
}

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> N>
BoundingBox<T> GeometryMeshGroup<T, N>::GetBoundingBox() const noexcept
{
    BoundingBox<T> ret;
    for(auto &it : submeshes)
        ret.Union(it.second.GetBoundingBox());
    return ret;
}

} // namespace AGZ::Mesh
