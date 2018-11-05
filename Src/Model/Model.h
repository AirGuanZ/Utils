#pragma once

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
struct GeometryMesh
{
    struct Vertex
    {
        Math::Vec3d pos;
        Math::Vec3d tex;
        Math::Vec3d nor;
    };

    std::vector<Vertex> vertices;

	/**
	 * @brief 自动平滑模型法线
	 * 
	 * @warning 算法非常糙，慎用
	 */
    GeometryMesh &SmoothenNormals();
};

/**
 * @brief 简单的几何模型组，为从名字到几何模型的映射
 */
struct GeometryMeshGroup
{
    std::map<Str8, GeometryMesh> submeshes;

	/**
	 * @brief 自动平滑模型法线
	 *
	 * @warning 算法非常糙，慎用
	 */
    GeometryMeshGroup &SmoothenNormals();
};

inline GeometryMesh &GeometryMesh::SmoothenNormals()
{
    // IMPROVE

    struct CompVec3d
    {
        bool operator()(const Math::Vec3d &lhs, const Math::Vec3d &rhs) const
        {
            return lhs.x < rhs.x ||
                  (lhs.x == rhs.x && lhs.y < rhs.y) ||
                  (lhs.xy() == rhs.xy() && lhs.z < rhs.z);
        }
    };

    std::map<Math::Vec3d, std::set<size_t>, CompVec3d> vtx2Nors;
    for(size_t i = 0; i < vertices.size(); ++i)
        vtx2Nors[vertices[i].pos].insert(i);

    for(auto &p : vtx2Nors)
    {
        auto avgNor = std::accumulate(p.second.begin(), p.second.end(), Math::Vec3d(0.0),
            [=](const Math::Vec3d &L, size_t R) { return L + this->vertices[R].nor; });
        if(avgNor.LengthSquare() < 0.001)
            continue;
        avgNor = avgNor.Normalize();
        for(auto i : p.second)
            vertices[i].nor = avgNor;
    }

    return *this;
}

inline GeometryMeshGroup &GeometryMeshGroup::SmoothenNormals()
{
    for(auto &p : submeshes)
        p.second.SmoothenNormals();
    return *this;
}

} // namespace AGZ::Model
