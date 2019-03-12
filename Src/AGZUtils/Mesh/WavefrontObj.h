#pragma once

#include <limits>
#include <map>
#include <type_traits>

#include "../Math/Vec3.h"
#include "../Math/Vec4.h"
#include "../String/StdStr.h"
#include "../Utils/FileSys.h"
#include "../Utils/Range.h"
#include "Mesh.h"

namespace AGZ::Mesh {

/**
 * @brief Wavefront OBJ类型文件的加载和解析
 *
 * 模型被分为两个层次，一个文件可以包含多个object（物体），一个object中可以有多个polygon group（组），每个group中则可以有多个三角形或四边形。
 * 顶点位置、发现、uv数据是以文件为单位进行管理的，polygon group中每个多边形存储的是该文件中顶点数据的下标。
 */
template<typename T>
class WavefrontObj
{
    static_assert(std::is_floating_point_v<T>, "Template parameter must be of floating-point type");

public:

    /** 顶点数据下标类型 */
    using Index = int32_t;

    /** 顶点数据下标的无效值 */
    static constexpr Index INDEX_NONE = (std::numeric_limits<Index>::max)();

    /**
     * @brief 物体
     * 
     * 一个物体可以包含多个polygon group（组），每个组有自己的名字，存储在object.name2Group中。
     */
    struct Object
    {
        /**
         * @brief polygon group
         * 
         * 一个组包含一系列三角形或四边形，通过group.faces访问。
         */
        struct Group
        {
            /**
             * @brief 一个三角形或四边形面
             */
            struct Face
            {
                /** 是三角形还是四边形，若是后者，则v[3]的值无效 */
                bool isTriangle;

                /** 多边形的顶点包含哪些数据下标 */
                struct FaceVertex
                {
                    Index pos;
                    Index tex;
                    Index nor;
                };

                /** 多边形的顶点下标值 */
                FaceVertex v[4];
            };

            /** 组中的所有多边形 */
            std::vector<Face> faces;
        };

        /** 取得具有指定名字的组 */
		const Group *FindGroup(std::string_view name) const
        {
            auto it = name2Group.find(std::string(name));
            if(it != name2Group.end())
                return &it->second;
            return nullptr;
        }

        /** 从名字到组的映射 */
        std::map<std::string, Group> name2Group;
    };

    /** 取得具有指定名字的物体 */
	const Object *FindObject(std::string_view name) const
    {
        auto it = name2Obj.find(std::string(name));
        if(it != name2Obj.end())
            return &it->second;
        return nullptr;
    }

    /** 清空已加载的所有数据 */
    void Clear()
    {
        vtxPos.clear();
        vtxTex.clear();
        vtxNor.clear();
        name2Obj.clear();
    }

    /** 加载指定的obj文件 */
    bool LoadFromFile(std::string_view filename)
    {
        std::string content;
        if(!FileSys::ReadTextFileRaw(filename, &content))
            return false;
        return LoadFromMemory(content);
    }

    /** 从字符串中加载obj内容 */
    bool LoadFromMemory(std::string_view content, bool ignoreUnknownLine = true) noexcept;

    /** 将包含的某个组转换为 GeometryMesh<T> 类型 */
    GeometryMesh<T> ToGeometryMesh(
        const typename Object::Group &grp, bool reverseNor = false, bool reverseTex = false) const;

    /** 将整个obj转换为一个 GeometryMeshGroup<T>，名字映射为“物体名-组名” */
    GeometryMeshGroup<T> ToGeometryMeshGroup(bool reverseNor = false, bool reverseTex = false) const;

    /** 顶点位置数组 */
    std::vector<Math::Vec3<T>> vtxPos;
    /** 顶点uv数组 */
    std::vector<Math::Vec3<T>> vtxTex;
    /** 顶点法线数组 */
    std::vector<Math::Vec3<T>> vtxNor;

    /** 从名字到物体的映射 */
    std::map<std::string, Object> name2Obj;

private:

    static typename Object::Group::Face::FaceVertex ParseVertexIndex(std::string_view s);
};

template<typename T>
bool WavefrontObj<T>::LoadFromMemory(std::string_view content, bool ignoreUnknownLine) noexcept
{
    Clear();

    // 通过obj()来获取当前正在parse的object
    // 通过grp()来获取当前正在parse的group

    Object                  *_curObj = nullptr;
    typename Object::Group  *_curGrp = nullptr;

    auto obj = [&]() -> Object&
    {
        if(!_curObj) _curObj = &name2Obj["Default"];
        return *_curObj;
    };

    auto grp = [&]() -> typename Object::Group&
    {
        if(!_curGrp) _curGrp = &obj().name2Group["Default"];
        return *_curGrp;
    };

    try
    {
        // 按\n拆分并剔除空行、注释行

        std::vector<std::string> lines;
        {
            std::vector<std::string> prelines;
            Split(content, "\n", std::back_inserter(prelines), true);
            for(auto &line : prelines)
            {
                if(!line.empty() && !StartsWith(line, "#"))
                    lines.push_back(Trim(line));
            }
        }
        
        for(const auto &line : lines)
        {
            if(StartsWith(line, "o "))
            {
                std::string name = Trim(line.substr(2));
                name2Obj.erase(name);
                _curObj = &name2Obj[name];
                _curGrp = nullptr;
                continue;
            }

            if(StartsWith(line, "g "))
            {
                std::string name = Trim(line.substr(2));
                obj().name2Group.erase(name);
                _curGrp = &obj().name2Group[name];
                continue;
            }

            //static const TScanner<char> vScanner("v {} {} {}");
            //if(T x, y, z; vScanner.Scan(line, x, y, z))
            //{
            //    vtxPos.push_back(Math::Vec3<T>(x, y, z));
            //    continue;
            //}
            if(StartsWith(line, "v "))
            {
                std::vector<std::string_view> ps;
                Split(line, std::back_inserter(ps));
                if(ps.size() != 4)
                    throw std::runtime_error("");
                vtxPos.push_back(Math::Vec3<T>(Parse<T>(ps[1]), Parse<T>(ps[2]), Parse<T>(ps[3])));
                continue;
            }

            //static const TScanner<char> vt2Scanner("vt {} {}");
            //if(T x, y; vt2Scanner.Scan(line, x, y))
            //{
            //    vtxTex.push_back(Math::Vec3<T>(x, y, 0));
            //    continue;
            //}

            //static const TScanner<char> vt3Scanner("vt {} {} {}");
            //if(T x, y, z; vt3Scanner.Scan(line, x, y, z))
            //{
            //    vtxTex.push_back(Math::Vec3<T>(x, y, z));
            //    continue;
            //}
            if(StartsWith(line, "vt "))
            {
                std::vector<std::string_view> ps;
                Split(line, std::back_inserter(ps));
                if(ps.size() == 3)
                    vtxTex.push_back(Math::Vec3<T>(Parse<T>(ps[1]), Parse<T>(ps[2]), 0));
                else if(ps.size() == 4)
                    vtxTex.push_back(Math::Vec3<T>(Parse<T>(ps[1]), Parse<T>(ps[2]), Parse<T>(ps[3])));
                else
                    throw std::runtime_error("");
                continue;
            }

            //static const TScanner<char> vnScanner("vn {} {} {}");
            //if(T x, y, z; vnScanner.Scan(line, x, y, z))
            //{
            //    vtxNor.push_back(Math::Vec3<T>(x, y, z));
            //    continue;
            //}
            if(StartsWith(line, "vn "))
            {
                std::vector<std::string_view> ps;
                Split(line, std::back_inserter(ps));
                if(ps.size() != 4)
                    throw std::runtime_error("");
                vtxNor.push_back(Math::Vec3<T>(Parse<T>(ps[1]), Parse<T>(ps[2]), Parse<T>(ps[3])));
                continue;
            }

            if(StartsWith(line, "f "))
            {
                std::vector<std::string> indices;
                Split(line.substr(2), std::back_inserter(indices));
                for(auto &idx : indices)
                    idx = Trim(idx);

                if(indices.size() < 3 || indices.size() > 4)
                    throw std::runtime_error("");

                typename Object::Group::Face face;
                for(size_t i = 0; i < indices.size(); ++i)
                {
                    auto v = ParseVertexIndex(indices[i]);
                    if(v.pos < 0) v.pos = Index(vtxPos.size()) + v.pos;
                    else if(v.pos != INDEX_NONE) --v.pos;
                    if(v.tex < 0) v.tex = Index(vtxTex.size()) + v.tex;
                    else if(v.tex != INDEX_NONE) --v.tex;
                    if(v.nor < 0) v.nor = Index(vtxNor.size()) + v.nor;
                    else if(v.nor != INDEX_NONE) --v.nor;
                    face.v[i] = v;
                }

                if(indices.size() == 3)
                    face.v[3].pos = face.v[3].tex = face.v[3].nor = INDEX_NONE;

                face.isTriangle = indices.size() != 4;

                grp().faces.push_back(face);
                continue;
            }

            if(!ignoreUnknownLine)
                throw std::runtime_error("");
        }
    }
    catch(...)
    {
        Clear();
        return false;
    }

    return true;
}

template<typename T>
typename WavefrontObj<T>::Object::Group::Face::FaceVertex
WavefrontObj<T>::ParseVertexIndex(std::string_view str)
{
    typename Object::Group::Face::FaceVertex ret = { INDEX_NONE, INDEX_NONE, INDEX_NONE };

    static const TScanner<char> minusIndexScanner("{}");
    if(int32_t idx; minusIndexScanner.Scan(str, idx))
    {
        ret.pos = idx;
        return ret;
    }

    static const TScanner<char> posAndTexScanner("{}/{}");
    if(int32_t pos, tex; posAndTexScanner.Scan(str, pos, tex))
    {
        ret.pos = pos;
        ret.tex = tex;
        return ret;
    }

    static const TScanner<char> posAndNorScanner("{}//{}");
    if(int32_t pos, nor; posAndNorScanner.Scan(str, pos, nor))
    {
        ret.pos = pos;
        ret.nor = nor;
        return ret;
    }

    static const TScanner<char> posAndTexAndNorScanner("{}/{}/{}");
    if(int32_t pos, nor, tex; posAndTexAndNorScanner.Scan(str, pos, tex, nor))
    {
        ret.pos = pos;
        ret.tex = tex;
        ret.nor = nor;
        return ret;
    }

    throw std::runtime_error("");
}

template<typename T>
GeometryMesh<T> WavefrontObj<T>::ToGeometryMesh(const typename Object::Group &grp, bool reverseNor, bool reverseTex) const
{
    std::vector<typename GeometryMesh<T>::Vertex> vtces;

    Math::Vec3<T> tB(T(1), T(0), T(0)), tC(T(0), T(1), T(0));
    if(reverseTex)
        std::swap(tB, tC);

    auto addTri = [&](const typename Object::Group::Face &f, const int (&js)[3])
    {
        size_t i = vtces.size();
        vtces.resize(vtces.size() + 3);

        for(int j = 0; j < 3; ++j)
            vtces[i + j].pos = vtxPos[f.v[js[j]].pos];

        if(f.v[js[0]].nor != INDEX_NONE && f.v[js[1]].nor != INDEX_NONE && f.v[js[2]].nor != INDEX_NONE)
        {
            for(int j = 0; j < 3; ++j)
                vtces[i + j].nor = vtxNor[f.v[js[j]].nor];
        }
        else
        {
            auto nor = Cross(
                vtces[i + 1].pos - vtces[i].pos, vtces[i + 2].pos - vtces[i].pos).Normalize();
            if(reverseNor)
                nor = -nor;
            for(int j = 0; j < 3; ++j)
                vtces[i + j].nor = nor;
        }

        if(f.v[js[0]].tex != INDEX_NONE && f.v[js[1]].tex != INDEX_NONE && f.v[js[2]].tex != INDEX_NONE)
        {
            for(int j = 0; j < 3; ++j)
                vtces[i + j].tex = vtxTex[f.v[js[j]].tex];
        }
        else
        {
            vtces[i].tex = Math::Vec3<T>(T(0));
            vtces[i + 1].tex = tB;
            vtces[i + 2].tex = tC;
        }
    };

    for(auto &f : grp.faces)
    {
        addTri(f, { 0, 1, 2 });
        if(!f.isTriangle)
            addTri(f, { 0, 2, 3 });
    }

    return GeometryMesh<T>{ vtces };
}

template<typename T>
GeometryMeshGroup<T> WavefrontObj<T>::ToGeometryMeshGroup(bool reverseNor, bool reverseTex) const
{
    std::map<std::string, GeometryMesh<T>> submeshes;
    for(auto &p : name2Obj)
    {
        for(auto &p2 : p.second.name2Group)
        {
            submeshes[p.first + "-" + p2.first] = ToGeometryMesh(p2.second, reverseNor, reverseTex);
        }
    }
    return GeometryMeshGroup<T>{ std::move(submeshes) };
}

} // namespace AGZ::Mesh
