#pragma once

#include <map>
#include <vector>

#include "../Misc/Common.h"
#include "../Utils/Math.h"
#include "../Utils/String.h"

AGZ_NS_BEG(AGZ::Model)

struct GeometryMesh
{
    struct Vertex
    {
        Math::Vec3d pos;
        Math::Vec3d tex;
        Math::Vec3d nor;
    };

    std::vector<Vertex> vertices;
};

struct GeometryMeshGroup
{
    std::map<Str8, GeometryMesh> submeshes;
};

AGZ_NS_END(AGZ::Model)
