#pragma once

#include <optional>

#include "../FileSys/Raw.h"
#include "Config.h"

AGZ_NS_BEG(AGZ)

inline ConfigGroup::ConfigGroup(std::unordered_map<Str8, const ConfigNode*> &&children)
    : children_(std::move(children))
{

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

inline ConfigArray::ConfigArray(std::vector<const ConfigNode*> &&content)
    : array_(std::move(content))
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

inline const ConfigValue &ConfigValue::AsValue() const
{
    return *this;
}

namespace ConfigImpl
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
            return StrAlgo::IsUnicodeWhitespace(c) || c == ',' || c == '=' || c == ';' || c == ')';
        });
        if(!tidx)
            return None;
        if(tidx == StrView8::NPOS)
            tidx = src.Length();
        Str8 s = src.Prefix(tidx);
        src = src.Slice(tidx);
        return Token{ TokenType::Name, std::move(s) };
    }

    std::unordered_map<Str8, const ConfigNode*> ParseGroupContent(std::list<Token> &toks, ObjArena<> &arena);

    inline const ConfigNode *ParseItemRight(std::list<Token> &toks, ObjArena<> &arena)
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

        if(t.type == TokenType::LeftPara)
        {
            toks.pop_front();

            std::vector<const ConfigNode*> content;

            if(toks.empty())
                throw Exception("");

            if(toks.front().type == TokenType::RightPara)
            {
                toks.pop_front();
                return arena.Create<ConfigArray>(std::move(content));
            }

            content.push_back(ParseItemRight(toks, arena));

            while(!toks.empty() && toks.front().type != TokenType::RightPara)
            {
                if(toks.front().type != TokenType::Comma)
                    throw Exception("");
                toks.pop_front();

                content.push_back(ParseItemRight(toks, arena));
            }

            if(toks.empty())
                throw Exception("");
            toks.pop_front();

            return arena.Create<ConfigArray>(std::move(content));
        }

        if(t.type == TokenType::String || t.type == TokenType::Name)
        {
            auto ret = arena.Create<ConfigValue>(std::move(t.str));
            toks.pop_front();
            return ret;
        }

        throw Exception("");
    }

    inline std::unordered_map<Str8, const ConfigNode*> ParseGroupContent(std::list<Token> &toks, ObjArena<> &arena)
    {
        std::unordered_map<Str8, const ConfigNode*> ret;

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

            ret[std::move(left)] = right;
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
        global_ = ConfigImpl::ParseConfig(src.AsView(), arena_);
        return true;
    }
    catch(...)
    {
        return false;
    }
}

inline bool Config::LoadFromFile(const Str8 &filename)
{
    WStr content;
    if(!FileSys::ReadTextFileRaw(filename.ToStdWString(), &content))
        return false;

    return LoadFromMemory(Str8(content));
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

AGZ_NS_END(AGZ)