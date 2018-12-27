#pragma once

#include <limits>
#include <map>
#include <type_traits>

#include "../Math/Vec3.h"
#include "../Math/Vec4.h"
#include "../String/String/String.h"
#include "../Utils/Range.h"

namespace AGZ::Mesh {

template<typename T>
class WavefrontObj
{
    static_assert(std::is_floating_point_v<T>, "Template parameter must be of floating-point type");

public:

    using Index = int32_t;
    static constexpr Index INDEX_NONE = (std::numeric_limits<Index>::max)();

    struct Object
    {
        struct Group
        {
            struct Face
            {
                bool isTriangle;

                struct FaceVertex
                {
                    Index pos;
                    Index tex;
                    Index nor;
                };
                FaceVertex v[4];
            };

            std::vector<Face> faces;
        };

        Option<const Group&> FindGroup(const Str8 &name) const
        {
            auto it = name2Group.find(name);
            if(it != name2Group.end())
                return Some(it->second);
            return None;
        }

        const std::map<Str8, Group> &GetAllGroups() const
        {
            return name2Group;
        }

        std::map<Str8, Group> name2Group;
    };

    Option<const Object&> FindObject(const Str8 &name) const
    {
        auto it = name2Obj_.find(name);
        if(it != name2Obj_.end())
            return Some(it->second);
        return None;
    }

    const std::map<Str8, Object> &GetAllObjects() const noexcept
    {
        return name2Obj_;
    }

    bool LoadFromFile(const Str8 &filename)
    {
        Str8 content;
        if(!FileSys::ReadTextFileRaw(filename, &content))
            return false;
        return LoadFromMemory(content);
    }

    bool LoadFromMemory(const Str8 &content, bool ignoreUnknownLine = true) noexcept;

private:

    static typename Object::Group::Face::FaceVertex ParseVertexIndex(const Str8 &s);

    std::vector<Math::Vec3<T>> vtxPos_;
    std::vector<Math::Vec3<T>> vtxTex_;
    std::vector<Math::Vec3<T>> vtxNor_;

    std::map<Str8, Object> name2Obj_;
};

template<typename T>
bool WavefrontObj<T>::LoadFromMemory(const Str8 &content, bool ignoreUnknownLine) noexcept
{
    vtxPos_.clear();
    vtxTex_.clear();
    vtxNor_.clear();
    name2Obj_.clear();

    // 通过obj()来获取当前正在parse的object
    // 通过grp()来获取当前正在parse的group

    Object                  *_curObj = nullptr;
    typename Object::Group  *_curGrp = nullptr;

    auto obj = [&]() -> Object&
    {
        if(!_curObj) _curObj = &name2Obj_["Default"];
        return *_curObj;
    };

    auto grp = [&]() -> typename Object::Group&
    {
        if(!_curGrp) _curGrp = &obj().name2Group["Default"];
        return _curGrp;
    };

    try
    {
        // 按\n拆分并剔除空行、注释行

        auto lines = content.Split("\n")
            | FilterMap([](const Str8 &line) -> Option<Str8>
            {
                Str8 ret = line.Trim();
                if(ret.Empty() || ret.StartsWith("#"))
                    return None;
                return ret;
            })
            | Collect<std::vector<Str8>>();
        
        for(const auto &line : lines)
        {
            static thread_local Regex8 oReg(
                R"___(o\s+&@{!\s}+&)___");
            static thread_local Regex8 gReg(
                R"___(g\s+&@{!\s}+&)___");
            static thread_local Regex8 vReg(
                R"___(v\s+&@{!\s}+&\s+&@{!\s}+&\s+&@{!\s}+&(\s+@{!\s}+)?&)___");
            static thread_local Regex8 vtReg(
                R"___(vt\s+&@{!\s}+&\s+&@{!\s}+&(\s+@{!\s}+)?&)___");
            static thread_local Regex8 vnReg(
                R"___(vn\s+&@{!\s}+&\s+&@{!\s}+&\s+&@{!\s}+&)___");

            if(auto m = oReg.Match(line))
            {
                Str8 k(m(0, 1));
                name2Obj_.erase(k);
                _curObj = &name2Obj_[k];
                continue;
            }

            if(auto m = gReg.Match(line))
            {
                Str8 k(m(0, 1));
                obj().name2Group.erase(k);
                _curGrp = &obj().name2Group[k];
                continue;
            }

            if(auto m = vReg.Match(line))
            {
                Math::Vec4<T> v = {
                    m(0, 1).template Parse<T>(),
                    m(2, 3).template Parse<T>(),
                    m(4, 5).template Parse<T>(),
                    m(5, 6).Empty() ? T(1) : m(5, 6).TrimLeft().template Parse<T>()
                };
                vtxPos_.push_back(v.xyz() / v.w);
                continue;
            }

            if(auto m = vtReg.Match(line))
            {
                Math::Vec3<T> vt = {
                    m(0, 1).template Parse<T>(),
                    m(2, 3).template Parse<T>(),
                    m(3, 4).Empty() ? T(0) : m(3, 4).TrimLeft().template Parse<T>()
                };
                vtxTex_.push_back(vt);
                continue;
            }

            if(auto m = vnReg.Match(line))
            {
                Math::Vec3<T> vn = {
                    m(0, 1).template Parse<T>(),
                    m(2, 3).template Parse<T>(),
                    m(4, 5).template Parse<T>()
                };
                vtxNor_.push_back(vn);
                continue;
            }

            if(line.StartsWith("f"))
            {
                auto indices = line.Slice(1).Split();
                if(indices.size() < 3 || indices.size() > 4)
                    throw std::runtime_error("");
                
                typename Object::Group::Face face;
                for(size_t i = 0; i < indices.size(); ++i)
                {
                    auto v = ParseVertexIndex(indices[i]);
                    if(v.vtx < 0) v.vtx = Index(vtxPos_.size()) + v.vtx; else --v.vtx;
                    if(v.tex < 0) v.tex = Index(vtxTex_.size()) + v.tex; else --v.tex;
                    if(v.nor < 0) v.nor = Index(vtxNor_.size()) + v.nor; else --v.nor;
                    face.v[i] = v;
                }
                
                if(indices.size() == 3)
                    face.v[3].pos = face.v[3].tex = face.v[3].nor = INDEX_NONE;
                
                grp().faces.push_back(face);
                continue;
            }

            if(!ignoreUnknownLine)
                throw std::runtime_error("");
        }
    }
    catch(...)
    {
        vtxPos_.clear();
        vtxTex_.clear();
        vtxNor_.clear();
        name2Obj_.clear();
        return false;
    }

    return true;
}

template<typename T>
typename WavefrontObj<T>::Object::Group::Face::FaceVertex
WavefrontObj<T>::ParseVertexIndex(const Str8 &str)
{
    typename Object::Group::Face::FaceVertex ret = { INDEX_NONE, INDEX_NONE, INDEX_NONE };

    static thread_local Regex8 reg0(R"___(-?\d+)___");
    if(auto m = reg0.Match(str); m)
    {
        ret.vtx = str.Parse<int32_t>();
        return ret;
    }

    static thread_local Regex8 reg1(R"___(&\d+&/&\d+&)___");
    if(auto m = reg1.Match(str); m)
    {
        ret.vtx = m(0, 1).Parse<int32_t>();
        ret.tex = m(2, 3).Parse<int32_t>();
        return ret;
    }

    static thread_local Regex8 reg2(R"___(&\d+&/&\d*&/&\d+&)___");
    if(auto m = reg2.Match(str); m)
    {
        ret.vtx = m(0, 1).Parse<int32_t>();
        ret.tex = m(2, 3).Empty() ? INDEX_NONE : (m(2, 3).Parse<int32_t>());
        ret.nor = m(4, 5).Parse<int32_t>();
        return ret;
    }

    throw std::runtime_error("");
}

} // namespace AGZ::Mesh
