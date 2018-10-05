#pragma once

#include <vector>

#include "../Utils/Math.h"
#include "../Utils/String.h"

AGZ_NS_BEG(AGZ::FileSys)

// See https://en.wikipedia.org/wiki/Wavefront_.obj_file
struct WavefrontObj
{
    using Vertex = Math::Vec4<double>;
    using TexCoord = Math::Vec4<double>;
    using Normal = Math::Vec3<double>;

    struct Face
    {
        // -1 means this index is inavailable
        int32_t vtx, tex, nor;
    };

    std::vector<Vertex> vertices;
    std::vector<TexCoord> texCoords;
    std::vector<Normal> normals;

    std::vector<Face> faces;
};

class WavefrontObjFile
{
public:

    WavefrontObj LoadFromObjFile(const WStr &filename);

    void SaveToObjFile(const WavefrontObj &obj, const WStr &filename);
};

inline WavefrontObj WavefrontObjFile::LoadFromObjFile(const WStr &filename)
{
    // TODO
    return WavefrontObj{ };
}

inline void WavefrontObjFile::SaveToObjFile(const WavefrontObj &obj, const WStr &filename)
{
    // TODO
}

AGZ_NS_END(AGZ::FileSys)
