#pragma once

#include <vector>

#include "../Utils/Math.h"
#include "../Utils/Range.h"
#include "../Utils/String.h"
#include "Raw.h"

AGZ_NS_BEG(AGZ::FileSys)

// See https://en.wikipedia.org/wiki/Wavefront_.obj_file
class WavefrontObj
{
public:

    using Vertex = Math::Vec4<double>;
    using TexCoord = Math::Vec3<double>;
    using Normal = Math::Vec3<double>;

    struct Index
    {
        int32_t vtx, tex, nor;
    };

    struct Face
    {
        // -1 means this index is inavailable
        Index indices[4];
    };

    std::vector<Vertex> vertices;
    std::vector<TexCoord> texCoords;
    std::vector<Normal> normals;

    std::vector<Face> faces;

    bool Empty() const
    {
        return vertices.empty() && texCoords.empty() &&
               normals.empty() && faces.empty();
    }

    void Clear()
    {
        vertices.clear();
        texCoords.clear();
        normals.clear();
        faces.clear();
    }
};

class WavefrontObjFile
{
public:

    static bool LoadFromObjFile(const WStr &filename, WavefrontObj *obj);

private:

    static WavefrontObj::Index ParseIndex(const WStrView &str);
};

inline bool WavefrontObjFile::LoadFromObjFile(const WStr &filename, WavefrontObj *obj)
{
    AGZ_ASSERT(obj && obj->Empty());

    try
    {
        WStr content;
        if(!ReadTextFileRaw(filename, &content))
            return false;

        auto lines = content.Split("\n")
            | Map([](const WStr &line) -> WStr { return line.Trim(); })
            | Filter([](const WStr &line) -> bool { return !line.Empty() && !line.StartsWith("#"); })
            | Collect<std::vector<WStr>>();

        for(const WStr &line : lines)
        {
            static thread_local  WRegex vReg(
                R"___(v\s+&@{!\s}+&\s+&@{!\s}+&\s+&@{!\s}+&\s+&(@{!\s}+)?&\s+)___");
            static thread_local WRegex vtReg(
                R"___(vt\s+&@{!\s}+&\s+&@{!\s}+&\s+&(@{!\s}+)?&\s+)___");
            static thread_local  WRegex vnReg(
                R"___(vn\s+&@{!\s}+&\s+&@{!\s}+&\s+&@{!\s}+&\s+)___");

            if(auto m = vReg.Match(line); m)
            {
                WavefrontObj::Vertex vtx;
                vtx.x = m(0, 1).Parse<double>();
                vtx.y = m(2, 3).Parse<double>();
                vtx.z = m(4, 5).Parse<double>();
                vtx.w = m(6, 7).Empty() ? 1.0 : m(6, 7).Parse<double>();
                obj->vertices.push_back(vtx);
                continue;
            }

            if(auto m = vtReg.Match(line); m)
            {
                WavefrontObj::TexCoord texCoord;
                texCoord.u = m(0, 1).Parse<double>();
                texCoord.v = m(2, 3).Parse<double>();
                texCoord.m = m(4, 5).Empty() ? 0.0 : m(4, 5).Parse<double>();
                obj->texCoords.push_back(texCoord);
                continue;
            }
            
            if(auto m = vnReg.Match(line); m)
            {
                WavefrontObj::Normal nor;
                nor.x = m(0, 1).Parse<double>();
                nor.y = m(2, 3).Parse<double>();
                nor.z = m(4, 5).Parse<double>();
                obj->normals.push_back(nor);
                continue;
            }

            if(line.StartsWith("f"))
            {
                auto indices = line.Slice(1).Split();
                if(indices.size() < 3 || indices.size() > 4)
                {
                    obj->Clear();
                    return false;
                }

                WavefrontObj::Face face;
                for(size_t i = 0; i < indices.size(); ++i)
                    face.indices[i] = ParseIndex(indices[i]);

                if(indices.size() == 3)
                {
                    face.indices[3].vtx = -1;
                    face.indices[3].tex = -1;
                    face.indices[3].nor = -1;
                }

                obj->faces.push_back(face);
                continue;
            }

            obj->Clear();
            return false;
        }
    }
    catch(...)
    {
        obj->Clear();
        return false;
    }

    return true;
}

inline WavefrontObj::Index WavefrontObjFile::ParseIndex(const WStrView &str)
{
    WavefrontObj::Index ret = { -1, -1, -1 };

    static thread_local WRegex reg0(R"___(\d+)___");
    if(auto m = reg0.Match(str); m)
    {
        ret.vtx = str.Parse<int32_t>();
        return ret;
    }

    static thread_local WRegex reg1(R"___(&\d+&/&\d+&)___");
    if(auto m = reg1.Match(str); m)
    {
        ret.vtx = m(0, 1).Parse<int32_t>();
        ret.tex = m(2, 3).Parse<int32_t>();
        return ret;
    }

    static thread_local WRegex reg2(R"___(&\d+&/&\d*&/&\d+&)___");
    if(auto m = reg2.Match(str); m)
    {
        ret.vtx = m(0, 1).Parse<int32_t>();
        ret.tex = m(2, 3).Empty() ? -1 : m(2, 3).Parse<int32_t>();
        ret.nor = m(4, 5).Parse<int32_t>();
        return ret;
    }

    throw ArgumentException("Invalid face index format");
}

AGZ_NS_END(AGZ::FileSys)
