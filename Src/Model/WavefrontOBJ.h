#pragma once

#include <map>
#include <vector>

#include "../Misc/Exception.h"
#include "../Utils/FileSys.h"
#include "../Utils/Math.h"
#include "../Utils/Range.h"
#include "../Utils/String.h"
#include "Model.h"

namespace AGZ::Model {

/**
 * @brief Wavefront OBJ模型加载结果
 */
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
class WavefrontObj
{
public:

    using Vertex   = Math::Vec4<T>; ///< 顶点类型
    using TexCoord = Math::Vec3<T>; ///< 纹理坐标类型
    using Normal   = Math::Vec3<T>; ///< 法线类型

    /**
     * 顶点索引
     */
    struct Index
    {
        int32_t vtx; ///< 位置索引
        int32_t tex; ///< 纹理坐标索引
        int32_t nor; ///< 法线索引
    };

    /**
     * 包含三个点的面为三角形，四个点则是四边形
     */
    struct Face
    {
        /**
         * 三角形三个顶点数据或四边形四个顶点的数据
         * 若indices[3].vtx < 0，则为三角形；否则为四边形
         */
        Index indices[4];
    };

    /**
     * @brief Obj模型数据
     */
    struct Obj
    {
        std::vector<Vertex>   vertices;  ///< 顶点数组
        std::vector<TexCoord> texCoords; ///< 纹理坐标数组
        std::vector<Normal>   normals;   ///< 法线数组
        std::vector<Face> faces;         ///< 面元数组

        /**
         * 转换为 GeometryMesh
         * 
         * Missing normals will be filled with Cross(B - A, C - A)
         *      Specify reverseNor = true to use Cross(C - A, B - A)
         * Missing texcoods will be filled with B <- (1, 0) and C <- (0, 1)
         *      Specify reverseTex = true to use B <- (0, 1) and C <- (1, 0)
         */
        GeometryMesh<T> ToGeometryMesh(bool reverseNor = false, bool reverseTex = false) const;
    };

    std::map<Str8, Obj> objs;

    /**
     * 是否不包含任何模型
     */
    bool Empty() const
    {
        return objs.empty();
    }

    /**
     * 清空所有加载的数据
     */
    void Clear()
    {
        objs.clear();
    }

    /**
     * 转换为 GeometryMeshGroup
     */
    GeometryMeshGroup<T> ToGeometryMeshGroup(bool reverseNor = false, bool reverseTex = false) const;
};

/**
 * @brief Wafefront模型加载器
 * 
 * 见https://en.wikipedia.org/wiki/Wavefront_.obj_file
 */
template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
class WavefrontObjFile
{
public:

    /**
     * 从文件中加载
     * 
     * @param filename 文件路径
     * @param objs 输出模型，不得为空
     * @param ignoreUnknownLine 是否忽略无法识别的行，缺省为true
     * 
     * @return 加载成功时返回true
     */
    static bool LoadFromObjFile(const Str8 &filename, WavefrontObj<T> *objs, bool ignoreUnknownLine = true);

    /**
     * 从文本中加载
     * 
     * @param content OBJ格式的文本字符串
     * @param objs 输出模型，不得为空
     * @param ignoreUnknownLine 是否忽略无法识别的行，缺省为true
     * 
     * @return 加载成功时返回true
     */
    static bool LoadFromMemory(const Str8 &content, WavefrontObj<T> *objs, bool ignoreUnknownLine = true);

private:

    static typename WavefrontObj<T>::Index ParseIndex(const StrView8 &str);
};

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> DN>
inline GeometryMesh<T> WavefrontObj<T, DN>::Obj::ToGeometryMesh(bool reverseNor, bool reverseTex) const
{
    std::vector<typename GeometryMesh<T>::Vertex> vtces;

    TexCoord tB(T(1), T(0), T(0)), tC(T(0), T(1), T(0));
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
            vtces[i].tex = TexCoord(T(0));
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

    return GeometryMesh<T>{ vtces };
}

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> DN>
inline GeometryMeshGroup<T> WavefrontObj<T, DN>::ToGeometryMeshGroup(bool reverseNor, bool reverseTex) const
{
    std::map<Str8, GeometryMesh<T>> submeshes;
    for(auto p : objs)
        submeshes[p.first] = p.second.ToGeometryMesh(reverseNor, reverseTex);
    return GeometryMeshGroup<T>{ submeshes };
}

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> DN>
inline bool WavefrontObjFile<T, DN>::LoadFromObjFile(const Str8 &filename, WavefrontObj<T> *objs, bool ignoreUnknownLine)
{
    AGZ_ASSERT(objs && objs->Empty());

    Str8 content;
    if(!FileSys::ReadTextFileRaw(filename, &content))
        return false;

    return LoadFromMemory(content, objs, ignoreUnknownLine);
}

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> DN>
inline bool WavefrontObjFile<T, DN>::LoadFromMemory(const Str8 &content, WavefrontObj<T> *objs, bool ignoreUnknownLine)
{
    AGZ_ASSERT(objs && objs->Empty());

    typename WavefrontObj<T>::Obj *cur = nullptr;
    auto checkCur = [&]()
    {
        if(!cur)
            cur = &objs->objs["Default"];
        return cur;
    };

    try
    {
        auto lines = content.Split("\n")
            | FilterMap([](const Str8 &line) -> std::optional<Str8>
              {
                  Str8 ret = line.Trim();
                  if(ret.Empty() || ret.StartsWith("#"))
                      return std::nullopt;
                  return ret;
              })
            | Collect<std::vector<Str8>>();

        for(const auto &line : lines)
        {
            static thread_local Regex8 oReg(
                R"___(o\s+&@{!\s}+&\s*)___");
            static thread_local Regex8 vReg(
                R"___(v\s+&@{!\s}+&\s+&@{!\s}+&\s+&@{!\s}+&(\s+@{!\s}+)?&\s*)___");
            static thread_local Regex8 vtReg(
                R"___(vt\s+&@{!\s}+&\s+&@{!\s}+&(\s+@{!\s}+)?&\s*)___");
            static thread_local Regex8 vnReg(
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
                typename WavefrontObj<T>::Vertex vtx;
                vtx.x = m(0, 1).template Parse<T>();
                vtx.y = m(2, 3).template Parse<T>();
                vtx.z = m(4, 5).template Parse<T>();
                vtx.w = m(5, 6).Empty() ? 1.0 : m(5, 6).TrimLeft().template Parse<T>();
                checkCur()->vertices.push_back(vtx);
                continue;
            }

            if(auto m = vtReg.Match(line); m)
            {
                typename WavefrontObj<T>::TexCoord texCoord;
                texCoord.u = m(0, 1).template Parse<T>();
                texCoord.v = m(2, 3).template Parse<T>();
                texCoord.m = m(3, 4).Empty() ? 0.0 : m(3, 4).TrimLeft().template Parse<T>();
                checkCur()->texCoords.push_back(texCoord);
                continue;
            }

            if(auto m = vnReg.Match(line); m)
            {
                typename WavefrontObj<T>::Normal nor;
                nor.x = m(0, 1).template Parse<T>();
                nor.y = m(2, 3).template Parse<T>();
                nor.z = m(4, 5).template Parse<T>();
                checkCur()->normals.push_back(nor.Normalize());
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

                typename  WavefrontObj<T>::Face face;
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

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> DN>
inline typename WavefrontObj<T>::Index WavefrontObjFile<T, DN>::ParseIndex(const StrView8 &str)
{
    typename WavefrontObj<T>::Index ret = { -1, -1, -1 };

    static thread_local Regex8 reg0(R"___(\d+)___");
    if(auto m = reg0.Match(str); m)
    {
        ret.vtx = str.Parse<int32_t>() - 1;
        return ret;
    }

    static thread_local Regex8 reg1(R"___(&\d+&/&\d+&)___");
    if(auto m = reg1.Match(str); m)
    {
        ret.vtx = m(0, 1).Parse<int32_t>() - 1;
        ret.tex = m(2, 3).Parse<int32_t>() - 1;
        return ret;
    }

    static thread_local Regex8 reg2(R"___(&\d+&/&\d*&/&\d+&)___");
    if(auto m = reg2.Match(str); m)
    {
        ret.vtx = m(0, 1).Parse<int32_t>() - 1;
        ret.tex = m(2, 3).Empty() ? -1 : (m(2, 3).Parse<int32_t>() - 1);
        ret.nor = m(4, 5).Parse<int32_t>() - 1;
        return ret;
    }

    throw ArgumentException("Invalid face index format");
}

} // namespace AGZ::Model
