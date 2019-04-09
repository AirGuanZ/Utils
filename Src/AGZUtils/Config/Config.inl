#pragma once

#include <optional>

#include "../FileSys/Raw.h"
#include "../Utils/String.h"
#include "Config.h"

namespace AGZ {

inline ConfigGroup::ConfigGroup(std::map<std::string, ConfigNode*> &&children)
    : children_(std::move(children))
{

}

inline void ConfigGroup::Expand(const std::map<std::string, ConfigNode*> &more)
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

inline const ConfigNode *ConfigGroup::FindSection(std::string_view k) const
{
    auto it = children_.find(std::string(k));
    return it != children_.end() ? it->second : nullptr;
}

inline const ConfigNode *ConfigGroup::Find(std::string_view k) const
{
    std::vector<std::string_view> sections;
    Split(k, ".", std::back_inserter(sections), false);
    //auto sections = k.Split(".");
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

inline const ConfigArray *ConfigGroup::FindArray(std::string_view k) const
{
    auto node = Find(k);
    if(!node)
        return nullptr;
    return node->IsArray() ? &node->AsArray() : nullptr;
}

inline const ConfigGroup *ConfigGroup::FindGroup(std::string_view k) const
{
    auto node = Find(k);
    if(!node)
        return nullptr;
    return node->IsGroup() ? &node->AsGroup() : nullptr;
}

inline const std::string *ConfigGroup::FindValue(std::string_view k) const
{
    auto node = Find(k);
    if(!node)
        return nullptr;
    return node->IsValue() ? &node->AsValue() : nullptr;
}

template<typename T, typename A>
std::optional<T> ConfigGroup::FindAndParse(std::string_view k, A &&parseParam) const
{
    if(auto v = FindValue(k))
    {
        try
        {
            return Parse<T>(*v, std::forward<A>(parseParam));
        }
        catch(...)
        {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

template<typename T>
std::optional<T> ConfigGroup::FindAndParse(std::string_view k) const
{
    if(auto v = FindValue(k))
    {
        try
        {
            return Parse<T>(*v);
        }
        catch(...)
        {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

inline const ConfigNode &ConfigGroup::operator[](std::string_view k) const
{
    auto node = Find(k);
    if(!node)
        throw ConfigNodeKeyNotFound("Key not found: " + std::string(k));
    return *node;
}

inline const ConfigGroup &ConfigGroup::AsGroup() const
{
    return *this;
}

inline std::string ConfigGroup::ToString() const
{
    std::string ret = "{";
    std::vector<std::string> mappedChildren;
    std::transform(begin(children_), end(children_), std::back_inserter(mappedChildren),
        [](auto &p)
    {
        return p.first + "=" + p.second->ToString() + ";";
    });
    ret.append(Join("", begin(mappedChildren), end(mappedChildren)));
    ret.push_back('}');
    return ret;
}

inline std::string ConfigGroup::ToPrettyString(const std::string &prefix, const std::string &delim, bool wrap) const
{
    std::string ret;
    if(wrap)
        ret = "{\n";
    std::string nprefix = wrap ? (prefix + delim) : prefix;
    for(auto &p : children_)
        ret.append(nprefix + p.first + " = " + p.second->ToPrettyString(nprefix, delim) + ";\n");
    if(wrap)
        ret.append(prefix + "}");
    return ret;
}

inline ConfigArray::ConfigArray(std::vector<const ConfigNode*> &&content, std::string tag)
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

inline std::string ConfigArray::ToString() const
{
    std::string ret = tag_ + "(";
    std::vector<std::string> mappedArray;
    std::transform(std::begin(array_), std::end(array_),
        std::back_inserter(mappedArray), [](auto node) { return node->ToString(); });
    ret.append(Join(",", std::begin(mappedArray), std::end(mappedArray)));
    ret.push_back(')');
    return ret;
}

inline std::string ConfigArray::ToPrettyString(const std::string &prefix, const std::string &delim, [[maybe_unused]] bool wrap) const
{
    std::string ret = tag_ + "(\n";
    std::string nprefix = prefix + delim;
    for(auto p : array_)
        ret.append(nprefix + p->ToPrettyString(nprefix, delim) + ",\n");
    ret.append(prefix + ")");
    return ret;
}

inline ConfigValue::ConfigValue(std::string str)
    : str_(std::move(str))
{

}

inline const std::string &ConfigValue::GetStr() const
{
    return str_;
}

inline const std::string &ConfigValue::operator*() const
{
    return str_;
}

inline const std::string &ConfigValue::AsValue() const
{
    return str_;
}

inline std::string ConfigValue::ToString() const
{
    return "\"" + str_ + "\"";
}

inline std::string ConfigValue::ToPrettyString(
    [[maybe_unused]] const std::string& prefix, [[maybe_unused]] const std::string& delim, [[maybe_unused]] bool wrap) const
{
    return ToString();
}

namespace Impl
{
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
        std::string str;
    };

    inline std::optional<Token> NextToken(std::string &src)
    {
        // Skip whitespaces and comments

        while(true)
        {
            src = TrimLeft(src);;
            if(StartsWith(src, "###"))
            {
                auto t = src.find("###", 3);
                if(t == std::string_view::npos)
                    return std::nullopt;
                src = src.substr(t + 3);
            }
            else if(StartsWith(src, "#"))
            {
                auto t = src.find("\n", 1);
                if(t == std::string_view::npos)
                    src = src.substr(0, 0);
                else
                    src = src.substr(t + 1);
            }
            else
                break;
        }

        if(src.empty())
            return std::nullopt;

        switch(src[0])
        {

        case '{':
            src = src.substr(1);
            return Token{ TokenType::LeftBrac, std::string() };

        case '}':
            src = src.substr(1);
            return Token{ TokenType::RightBrac, std::string() };

        case '=':
            src = src.substr(1);
            return Token{ TokenType::Equal, std::string() };

        case ';':
            src = src.substr(1);
            return Token{ TokenType::Semicolon, std::string() };

        case ',':
            src = src.substr(1);
            return Token{ TokenType::Comma, std::string() };

        case '(':
            src = src.substr(1);
            return Token{ TokenType::LeftPara, std::string() };

        case ')':
            src = src.substr(1);
            return Token{ TokenType::RightPara, std::string() };

        default:
            break;
        }

        if(StartsWith(src, "\""))
        {
            size_t nextQuote = 1;
            while(nextQuote < src.size())
            {
                if(src[nextQuote] == '\"' && src[nextQuote - 1] != '\\')
                    break;
                ++nextQuote;
            }
            if(nextQuote >= src.size())
                return std::nullopt;
            std::string retStr = src.substr(1, nextQuote - 1);
            src = src.substr(nextQuote + 1);
            return Token{ TokenType::String, std::move(retStr) };
        }

        auto tidx = src.find_first_of(" \n\r\t\f\v,=;()#");
        if(!tidx)
            return std::nullopt;
        if(tidx == std::string_view::npos)
            tidx = src.length();
        std::string s = src.substr(0, tidx);
        src = src.substr(tidx);
        return Token{ TokenType::Name, s };
    }

    std::map<std::string, ConfigNode*> ParseGroupContent(std::list<Token> &toks, ObjArena<> &arena);

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

        std::string arrTag;
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

    inline std::map<std::string, ConfigNode*> ParseGroupContent(std::list<Token> &toks, ObjArena<> &arena)
    {
        std::map<std::string, ConfigNode*> ret;

        while(!toks.empty() && toks.front().type != TokenType::RightBrac)
        {
            if(toks.front().type != TokenType::Name)
                throw Exception("");

            std::string left = std::move(toks.front().str);
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

    inline ConfigGroup *ParseConfig(std::string src, ObjArena<> &arena)
    {
        std::list<Token> toks;
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

inline bool Config::LoadFromMemory(std::string_view src)
{
    Clear();
    try
    {
        global_ = Impl::ParseConfig(std::string(src), arena_);
        return true;
    }
    catch(...)
    {
        return false;
    }
}

inline bool Config::LoadFromFile(std::string_view filename)
{
    std::string content;
    if(!FileSys::ReadTextFileRaw(filename, &content))
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

inline std::string Config::ToPrettyForm(std::string_view src)
{
    Config config;
    if(!config.LoadFromMemory(src))
        return "";
    return config.ToPrettyString();
}

} // namespace AGZ
