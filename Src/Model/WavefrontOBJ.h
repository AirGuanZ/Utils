#pragma once

#include <map>
#include <vector>

#include "../Utils/Math.h"
#include "../Utils/Range.h"
#include "../Utils/String.h"
#include "../Utils/FileSys.h"
#include "Model.h"

AGZ_NS_BEG(AGZ::Model)

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

    struct Obj
    {
        std::vector<Vertex> vertices;
        std::vector<TexCoord> texCoords;
        std::vector<Normal> normals;

        std::vector<Face> faces;

        // Convert self to GeometryMesh
        // Missing normals will be filled with Cross(B - A, C - A)
        //      Specify reverseNor = true to use Cross(C - A, B - A)
        // Missing texcoods will be filled with B <- (1, 0) and C <- (0, 1)
        //      Specify reverseTex = true to use B <- (0, 1) and C <- (1, 0)
        GeometryMesh ToGeometryMesh(bool reverseNor = false, bool reverseTex = false) const;
    };

    std::map<Str8, Obj> objs;

    bool Empty() const
    {
        return objs.empty();
    }

    void Clear()
    {
        objs.clear();
    }

    GeometryMeshGroup ToGeometryMeshGroup(bool reverseNor = false, bool reverseTex = false) const;
};

class WavefrontObjFile
{
public:

    static bool LoadFromObjFile(const WStr &filename, WavefrontObj *objs, bool ignoreUnknownLine = true);

    static bool LoadFromMemory(const WStr &content, WavefrontObj *objs, bool ignoreUnknownLine = true);

private:

    static WavefrontObj::Index ParseIndex(const WStrView &str);
};

inline GeometryMesh WavefrontObj::Obj::ToGeometryMesh(bool reverseNor, bool reverseTex) const
{
    std::vector<GeometryMesh::Vertex> vtces;

    TexCoord tB(1.0, 0.0, 0.0), tC(0.0, 1.0, 0.0);
    if(reverseTex)
        std::swap(tB, tC);

    auto addTri = [&](const Face &f, const int (&js)[3])
    {
        size_t i = vtces.size();
        vtces.resize(vtces.size() + 3);

        for(int j = 0; j < 3; ++j)
            vtces[i + j].pos = vertices[f.indices[js[j]].vtx].xyz() / vertices[f.indices[js[j]].vtx].w;

        if(f.indices[0].nor >= 0 && f.indices[1].nor >= 0 && f.indices[2].nor >= 0)
        {
            for(int j = 0; j < 3; ++j)
                vtces[i + j].nor = normals[f.indices[js[j]].nor];
        }
        else
        {
            Normal nor = Cross(
                vtces[i + 1].pos - vtces[i].pos, vtces[i + 2].pos - vtces[i].pos).Normalize();
            if(reverseNor)
                nor = -nor;
            for(int j = 0; j < 3; ++j)
                vtces[i + j].nor = nor;
        }

        if(f.indices[js[0]].tex >= 0 && f.indices[js[1]].tex >= 0 && f.indices[js[2]].tex >= 0)
        {
            for(int j = 0; j < 3; ++j)
                vtces[i + j].tex = texCoords[f.indices[js[j]].tex];
        }
        else
        {
            vtces[i].tex = TexCoord(0.0);
            vtces[i + 1].tex = tB;
            vtces[i + 2].tex = tC;
        }
    };

    for(auto &f : faces)
    {
        addTri(f, { 0, 1, 2 });
        if(f.indices[3].vtx >= 0)
            addTri(f, { 0, 2, 3 });
    }

    return GeometryMesh{ vtces };
}

inline GeometryMeshGroup WavefrontObj::ToGeometryMeshGroup(bool reverseNor, bool reverseTex) const
{
    std::map<Str8, GeometryMesh> submeshes;
    for(auto p : objs)
        submeshes[p.first] = p.second.ToGeometryMesh(reverseNor, reverseTex);
    return GeometryMeshGroup{ submeshes };
}

inline bool WavefrontObjFile::LoadFromObjFile(const WStr &filename, WavefrontObj *objs, bool ignoreUnknownLine)
{
    AGZ_ASSERT(objs && objs->Empty());

    WStr content;
    if(!FileSys::ReadTextFileRaw(filename, &content))
        return false;

    return LoadFromMemory(content, objs, ignoreUnknownLine);
}

inline bool WavefrontObjFile::LoadFromMemory(const WStr &content, WavefrontObj *objs, bool ignoreUnknownLine)
{
    AGZ_ASSERT(objs && objs->Empty());

    WavefrontObj::Obj *cur = nullptr;
    auto checkCur = [&]()
    {
        if(!cur)
            cur = &objs->objs["Default"];
        return cur;
    };

    try
    {
        auto lines = content.Split("\n")
            | FilterMap([](const WStr &line) -> std::optional<WStr>
              {
                  WStr ret = line.Trim();
                  if(ret.Empty() || ret.StartsWith("#"))
                      return std::nullopt;
                  return ret;
              })
            | Collect<std::vector<WStr>>();

        for(const WStr &line : lines)
        {
            static thread_local WRegex oReg(
                R"___(o\s+&@{!\s}+&\s*)___");
            static thread_local WRegex vReg(
                R"___(v\s+&@{!\s}+&\s+&@{!\s}+&\s+&@{!\s}+&(\s+@{!\s}+)?&\s*)___");
            static thread_local WRegex vtReg(
                R"___(vt\s+&@{!\s}+&\s+&@{!\s}+&(\s+@{!\s}+)?&\s*)___");
            static thread_local WRegex vnReg(
                R"___(vn\s+&@{!\s}+&\s+&@{!\s}+&\s+&@{!\s}+&\s*)___");

            if(auto m = oReg.Match(line); m)
            {
                Str8 key(m(0, 1));
                objs->objs.erase(key);
                cur = &objs->objs[key];
                continue;
            }

            if(auto m = vReg.Match(line); m)
            {
                WavefrontObj::Vertex vtx;
                vtx.x = m(0, 1).Parse<double>();
                vtx.y = m(2, 3).Parse<double>();
                vtx.z = m(4, 5).Parse<double>();
                vtx.w = m(5, 6).Empty() ? 1.0 : m(5, 6).TrimLeft().Parse<double>();
                checkCur()->vertices.push_back(vtx);
                continue;
            }

            if(auto m = vtReg.Match(line); m)
            {
                WavefrontObj::TexCoord texCoord;
                texCoord.u = m(0, 1).Parse<double>();
                texCoord.v = m(2, 3).Parse<double>();
                texCoord.m = m(4, 5).Empty() ? 0.0 : m(4, 5).TrimLeft().Parse<double>();
                checkCur()->texCoords.push_back(texCoord);
                continue;
            }

            if(auto m = vnReg.Match(line); m)
            {
                WavefrontObj::Normal nor;
                nor.x = m(0, 1).Parse<double>();
                nor.y = m(2, 3).Parse<double>();
                nor.z = m(4, 5).Parse<double>();
                checkCur()->normals.push_back(nor);
                continue;
            }

            if(line.StartsWith("f"))
            {
                auto indices = line.Slice(1).Split();
                if(indices.size() < 3 || indices.size() > 4)
                {
                    objs->Clear();
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

                checkCur()->faces.push_back(face);
                continue;
            }

            if(!ignoreUnknownLine)
            {
                objs->Clear();
                return false;
            }
        }
    }
    catch(...)
    {
        objs->Clear();
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
        ret.vtx = str.Parse<int32_t>() - 1;
        return ret;
    }

    static thread_local WRegex reg1(R"___(&\d+&/&\d+&)___");
    if(auto m = reg1.Match(str); m)
    {
        ret.vtx = m(0, 1).Parse<int32_t>() - 1;
        ret.tex = m(2, 3).Parse<int32_t>() - 1;
        return ret;
    }

    static thread_local WRegex reg2(R"___(&\d+&/&\d*&/&\d+&)___");
    if(auto m = reg2.Match(str); m)
    {
        ret.vtx = m(0, 1).Parse<int32_t>() - 1;
        ret.tex = m(2, 3).Empty() ? -1 : (m(2, 3).Parse<int32_t>() - 1);
        ret.nor = m(4, 5).Parse<int32_t>() - 1;
        return ret;
    }

    throw ArgumentException("Invalid face index format");
}

AGZ_NS_END(AGZ::Model)
