#pragma once

#include <optional>

#include "../FileSys/Raw.h"
#include "Config.h"

namespace AGZ {

inline ConfigGroup::ConfigGroup(std::unordered_map<Str8, ConfigNode*> &&children)
    : children_(std::move(children))
{

}

inline void ConfigGroup::Expand(const std::unordered_map<Str8, ConfigNode*> &more)
{
    for(auto &moreIt : more)
    {
        auto it = children_.find(moreIt.first);
        if(it == children_.end() || !it->second->IsGroup() || !moreIt.second->IsGroup())
            children_[moreIt.first] = moreIt.second;
        else
            dynamic_cast<ConfigGroup*>(it->second)->Expand(moreIt.second->AsGroup().GetChildren());
    }
}

inline const ConfigNode *ConfigGroup::FindSection(const StrView8 &k) const
{
    auto it = children_.find(k);
    return it != children_.end() ? it->second : nullptr;
}

inline const ConfigNode *ConfigGroup::Find(const Str8 &k) const
{
    return Find(k.AsView());
}

inline const ConfigNode *ConfigGroup::Find(const StrView8 &k) const
{
    auto sections = k.Split(".");
    const ConfigGroup *grp = this;

    for(size_t i = 0, end = sections.size() - 1; i < end; ++i)
    {
        auto node = grp->FindSection(sections[i]);
        if(!node)
            return nullptr;
        grp = &node->AsGroup();
    }

    return grp->FindSection(sections.back());
}

inline const ConfigNode &ConfigGroup::operator[](const Str8 &k) const
{
    return (*this)[k.AsView()];
}

inline const ConfigNode &ConfigGroup::operator[](const StrView8 &k) const
{
    auto node = Find(k);
    if(!node)
        throw ConfigNodeKeyNotFound(("Key not found: " + k).ToStdString());
    return *node;
}

inline const ConfigGroup &ConfigGroup::AsGroup() const
{
    return *this;
}

inline ConfigArray::ConfigArray(std::vector<const ConfigNode*> &&content, Str8 tag)
    : array_(std::move(content)), tag_(std::move(tag))
{
    
}

inline const ConfigNode *ConfigArray::At(size_t idx) const
{
    return idx < array_.size() ? array_[idx] : nullptr;
}

inline const ConfigNode &ConfigArray::operator[](size_t idx) const
{
    return *array_[idx];
}

inline size_t ConfigArray::Size() const
{
    return array_.size();
}

inline const ConfigArray &ConfigArray::AsArray() const
{
    return *this;
}

inline ConfigValue::ConfigValue(Str8 &&str)
    : str_(std::move(str))
{

}

inline const Str8 &ConfigValue::GetStr() const
{
    return str_;
}

inline const Str8 &ConfigValue::operator*() const
{
    return str_;
}

inline const Str8 &ConfigValue::AsValue() const
{
    return str_;
}

namespace Impl
{
    template<typename T>
    using Option = std::optional<T>;

    constexpr std::nullopt_t None = std::nullopt;

    enum class TokenType
    {
        Name, String,
        Equal, Semicolon, Comma,
        LeftBrac, RightBrac,
        LeftPara, RightPara,
    };

    struct Token
    {
        TokenType type;
        Str8 str;
    };

    inline Option<Token> NextToken(StrView8 &src)
    {
        // Skip whitespaces and comments

        while(true)
        {
            src = src.TrimLeft();
            if(src.StartsWith("#"))
            {
                auto t = src.Find("\n");
                if(t == StrView8::NPOS)
                    src = src.Slice(0, 0);
                else
                    src = src.Slice(t + 1);
            }
            else
                break;
        }

        if(src.Empty())
            return None;

        switch(*src.CodePoints().begin())
        {

        case '{':
            src = src.Slice(1);
            return Token{ TokenType::LeftBrac, Str8() };

        case '}':
            src = src.Slice(1);
            return Token{ TokenType::RightBrac, Str8() };

        case '=':
            src = src.Slice(1);
            return Token{ TokenType::Equal, Str8() };

        case ';':
            src = src.Slice(1);
            return Token{ TokenType::Semicolon, Str8() };

        case ',':
            src = src.Slice(1);
            return Token{ TokenType::Comma, Str8() };

        case '(':
            src = src.Slice(1);
            return Token{ TokenType::LeftPara, Str8() };

        case ')':
            src = src.Slice(1);
            return Token{ TokenType::RightPara, Str8() };

        default:
            break;
        }

        if(src.StartsWith("\""))
        {
            static thread_local Regex8 regex(R"__("&((@{!"}|\\")*@{!\\})?&")__");
            auto m = regex.SearchPrefix(src);
            if(!m)
                return None;
            src = src.Slice(m[1] + 1);
            return Token{ TokenType::String, m(0, 1) };
        }

        auto tidx = src.FindCPIf([](auto c)
        {
            return StrAlgo::IsUnicodeWhitespace(c) || c == ',' || c == '=' || c == ';' || c == ')' || c == '(';
        });
        if(!tidx)
            return None;
        if(tidx == StrView8::NPOS)
            tidx = src.Length();
        Str8 s = src.Prefix(tidx);
        src = src.Slice(tidx);
        return Token{ TokenType::Name, std::move(s) };
    }

    std::unordered_map<Str8, ConfigNode*> ParseGroupContent(std::list<Token> &toks, ObjArena<> &arena);

    inline ConfigNode *ParseItemRight(std::list<Token> &toks, ObjArena<> &arena)
    {
        if(toks.empty())
            throw Exception("");

        auto &t = toks.front();
        if(t.type == TokenType::LeftBrac)
        {
            toks.pop_front();
            
            auto content = ParseGroupContent(toks, arena);

            if(toks.empty() || toks.front().type != TokenType::RightBrac)
                throw Exception("");
            toks.pop_front();

            return arena.Create<ConfigGroup>(std::move(content));
        }

        Str8 arrTag;

        if(t.type == TokenType::Name)
        {
            auto name = std::move(t.str);
            toks.pop_front();

            if(toks.empty() || toks.front().type != TokenType::LeftPara)
                return arena.Create<ConfigValue>(std::move(name));
            arrTag = std::move(name);
        }
        else if(t.type == TokenType::String)
        {
            auto ret = arena.Create<ConfigValue>(std::move(t.str));
            toks.pop_front();
            return ret;
        }

        if(toks.front().type == TokenType::LeftPara)
        {
            toks.pop_front();

            std::vector<const ConfigNode*> content;

            if(toks.empty())
                throw Exception("");

            if(toks.front().type == TokenType::RightPara)
            {
                toks.pop_front();
                return arena.Create<ConfigArray>(std::move(content), std::move(arrTag));
            }

            content.push_back(ParseItemRight(toks, arena));

            while(!toks.empty() && toks.front().type != TokenType::RightPara)
            {
                if(toks.front().type != TokenType::Comma)
                    throw Exception("");
                toks.pop_front();

                if(toks.empty() || toks.front().type == TokenType::RightPara)
                    break;

                content.push_back(ParseItemRight(toks, arena));
            }

            if(toks.empty())
                throw Exception("");
            toks.pop_front();

            return arena.Create<ConfigArray>(std::move(content), std::move(arrTag));
        }

        throw Exception("");
    }

    inline std::unordered_map<Str8, ConfigNode*> ParseGroupContent(std::list<Token> &toks, ObjArena<> &arena)
    {
        std::unordered_map<Str8, ConfigNode*> ret;

        while(!toks.empty() && toks.front().type != TokenType::RightBrac)
        {
            if(toks.front().type != TokenType::Name)
                throw Exception("");

            Str8 left = std::move(toks.front().str);
            toks.pop_front();

            if(toks.empty() || toks.front().type != TokenType::Equal)
                throw Exception("");
            toks.pop_front();

            auto right = ParseItemRight(toks, arena);

            if(toks.empty() || toks.front().type != TokenType::Semicolon)
                throw Exception("");
            toks.pop_front();

            auto it = ret.find(left);
            if(it == ret.end() || !right->IsGroup() || !it->second->IsGroup())
                ret[std::move(left)] = right;
            else
                dynamic_cast<ConfigGroup*>(it->second)->Expand(right->AsGroup().GetChildren());
        }

        return ret;
    }

    inline ConfigGroup *ParseConfig(const StrView8 &_src, ObjArena<> &arena)
    {
        std::list<Token> toks;
        StrView8 src = _src;
        for(;;)
        {
            auto tok = NextToken(src);
            if(!tok)
                break;
            toks.push_back(*tok);
        }

        auto um = ParseGroupContent(toks, arena);
        if(!toks.empty())
            throw Exception("");
        return arena.Create<ConfigGroup>(std::move(um));
    }
}

inline bool Config::LoadFromMemory(const Str8 &src)
{
    Clear();
    try
    {
        global_ = Impl::ParseConfig(src.AsView(), arena_);
        return true;
    }
    catch(...)
    {
        return false;
    }
}

inline bool Config::LoadFromFile(const Str8 &filename)
{
    Str8 content;
    if(!FileSys::ReadTextFileRaw(filename.ToStdWString(), &content))
        return false;
    return LoadFromMemory(content);
}

inline bool Config::IsAvailable() const
{
    return global_ != nullptr;
}

inline void Config::Clear()
{
    arena_.Clear();
    global_ = nullptr;
}

inline const ConfigGroup &Config::Root() const
{
    AGZ_ASSERT(global_);
    return *global_;
}

} // namespace AGZ
