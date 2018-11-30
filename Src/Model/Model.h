#pragma once

#include <algorithm>
#include <map>
#include <numeric>
#include <set>
#include <vector>

#include "../Misc/Common.h"
#include "../Utils/Math.h"
#include "../Utils/String.h"

namespace AGZ::Model {

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
};

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> DN>
inline GeometryMesh<T> &GeometryMesh<T, DN>::SmoothenNormals()
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

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> DN>
inline GeometryMeshGroup<T> &GeometryMeshGroup<T, DN>::SmoothenNormals()
{
    for(auto &p : submeshes)
        p.second.SmoothenNormals();
    return *this;
}

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> DN>
inline GeometryMesh<T> GeometryMeshGroup<T, DN>::MergeAllSubmeshes() const
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

} // namespace AGZ::Model
