#pragma once

#include <vector>
#include <unordered_map>

#include "../Alloc/ObjArena.h"
#include "../Misc/Common.h"
#include "../Misc/Exception.h"
#include "../Misc/Uncopiable.h"
#include "../Utils/String.h"

namespace AGZ {

/*
window =
{
    title = "My Application";
    size = { w = 640; h = 480; };
    pos = { x = 350; y = 250; };
};

list = ( ( "abc", 123, true ), 1.234, ( ) );

books = ({
        title = "Treasure Island";
        author = "Robert Louis Stevenson";
        price = 29.95;
        qty = 5;
    },
    {
        title = "Snow Crash";
        author = "Neal Stephenson";
        price = 9.99;
        qty = 8;
    });

misc =
{
    pi = 3.141592654;
    bigint = 9223372036854775807L;
    columns = ["Last Name", "First Name", "MI"];
    bitmask = 0x1FC3;
    umask = 0027;
};
*/

class ConfigGroup;
class ConfigArray;
class ConfigValue;

AGZ_NEW_EXCEPTION(ConfigNodeKeyNotFound);
AGZ_NEW_EXCEPTION(ConfigNodeInvalidCasting);

class ConfigNode : public Uncopiable
{
public:

    virtual ~ConfigNode() = default;

    virtual const ConfigGroup &AsGroup() const { throw ConfigNodeInvalidCasting("ConfigASTNode: invalid casting"); }
    virtual const ConfigArray &AsArray() const { throw ConfigNodeInvalidCasting("ConfigASTNode: invalid casting"); }
    virtual const Str8        &AsValue() const { throw ConfigNodeInvalidCasting("ConfigASTNode: invalid casting"); }
};

class ConfigGroup : public ConfigNode
{
    std::unordered_map<Str8, const ConfigNode*> children_;

    const ConfigNode *FindSection(const StrView8 &k) const;

public:

    explicit ConfigGroup(std::unordered_map<Str8, const ConfigNode*> &&children);

    const ConfigNode *Find(const Str8 &k) const;
    const ConfigNode *Find(const StrView8 &k) const;

    const ConfigNode &operator[](const Str8 &k) const;
    const ConfigNode &operator[](const StrView8 &k) const;

    const ConfigGroup &AsGroup() const override;
};

class ConfigArray : public ConfigNode
{
    std::vector<const ConfigNode*> array_;

public:

    explicit ConfigArray(std::vector<const ConfigNode*> &&content);

    const ConfigNode *At(size_t idx) const;

    const ConfigNode &operator[](size_t idx) const;

    size_t Size() const;

    const ConfigArray &AsArray() const override;
};

class ConfigValue : public ConfigNode
{
    Str8 str_;

public:

    explicit ConfigValue(Str8 &&str);

    const Str8 &GetStr() const;

    const Str8 &operator*() const;

    const Str8 &AsValue() const override;
};

class Config : public Uncopiable
{
    ObjArena<> arena_;

    const ConfigGroup *global_ = nullptr;

public:

    bool LoadFromMemory(const Str8 &src);
    bool LoadFromFile(const Str8 &filename);
    bool IsAvailable() const;
    void Clear();

    const ConfigGroup &Root() const;
};

} // namespace AGZ

#include "Config.inl"
