#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "../Alloc/ObjArena.h"
#include "../Misc/Exception.h"

namespace AGZ {

class ConfigGroup;
class ConfigArray;
class ConfigValue;

AGZ_NEW_EXCEPTION(ConfigNodeKeyNotFound,    Exception);
AGZ_NEW_EXCEPTION(ConfigNodeInvalidCasting, Exception);

/**
 * @brief 配置参数树中的节点
 * 
 * 节点内容分为三种类型：
 * - Group 参数集合
 * - Array 参数数组
 * - Value 具体的参数值
 * 
 * @warning 节点的实际类型在解析配置文件时就已经确定，尝试将获得类型不正确的内容会导致异常
 */
class ConfigNode : public Uncopiable
{
public:

    virtual ~ConfigNode() = default;

    /**
     * 将this转换为参数集合
     * @exception ConfigNodeInvalidCasting this并不是参数集合时抛出
     */
    virtual const ConfigGroup &AsGroup() const { throw ConfigNodeInvalidCasting("ConfigASTNode: invalid casting"); }
    /**
     * 将this转换为参数数组
     * @exception ConfigNodeInvalidCasting this并不是参数数组时抛出
     */
    virtual const ConfigArray &AsArray() const { throw ConfigNodeInvalidCasting("ConfigASTNode: invalid casting"); }
    /**
     * 将this转换为参数值
     * @exception ConfigNodeInvalidCasting this并不是参数值时抛出
     */
    virtual const std::string &AsValue() const { throw ConfigNodeInvalidCasting("ConfigASTNode: invalid casting"); }

    /**
     * 转为value并parse
     */
    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    T Parse() const { return AGZ::Parse<T>(AsValue()); }

    /**
     * 转为value并parse
     */
    template<typename T, typename A, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    T Parse(A &&param) const { return Parse<T>(AsValue(), std::forward<A>(param)); }

    /**
     * 尝试将this转换为参数集合，类型不匹配时返回nullptr
     */
    virtual const ConfigGroup *TryAsGroup() const noexcept { return nullptr; }

    /**
     * 尝试将this转换为参数数组，类型不匹配时返回nullptr
     */
    virtual const ConfigArray *TryAsArray() const noexcept { return nullptr; }

    /**
     * 尝试将this转换为参数值，类型不匹配时返回nullptr
     */
    virtual const std::string *TryAsValue() const noexcept { return nullptr; }

    //! 是否是Group类型
    virtual bool IsGroup() const noexcept { return false; }
    //! 是否是Array类型
    virtual bool IsArray() const noexcept { return false; }
    //! 是否是Value类型
    virtual bool IsValue() const noexcept { return false; }

    /**
     * 转换为描述性字符串
     */
    virtual std::string ToString() const = 0;
};

/**
 * @brief 配置参数集合
 */
class ConfigGroup : public ConfigNode
{
    std::unordered_map<std::string, ConfigNode*> children_;

    const ConfigNode *FindSection(std::string_view k) const;

public:

    explicit ConfigGroup(std::unordered_map<std::string, ConfigNode*> &&children);

    /**
     * 扩充内容
     * 
     * - 若more中内容和children_无名字相同者，则直接合并
     * - 对名字相同者，若两个都是group，则递归地扩充该group
     * - 对其他情况，用more中的同名ConfigNode覆盖children_中的
     */
    void Expand(const std::unordered_map<std::string, ConfigNode*> &more);

    //! 取得所有内容
    const std::unordered_map<std::string, ConfigNode*> &GetChildren() const { return children_; }

    /**
     * 查找具有指定路径的配置参数值
     * 
     * @param k 带查找的参数路径，用“.”作为路径分隔符
     */
    const ConfigNode *Find(std::string_view k) const;

    /**
     * 查找具有指定路径的array，路径不存在或类型不正确时返回nullptr
     */
    const ConfigArray *FindArray(std::string_view k) const;

    /**
     * 查找具有指定路径的group，路径不存在或类型不正确时返回nullptr
     */
    const ConfigGroup *FindGroup(std::string_view k) const;

    /**
     * 查找具有指定路径的value，路径不存在或类型不正确时返回nullptr
     */
    const std::string *FindValue(std::string_view k) const;

    /**
     * 查找指定路径中的值，并parse为数值类型
     * 
     * @param k 查找路径
     * @param parseParam parse用的参数
     * 
     * 查找失败或parse失败时返回std::nullopt
     */
    template<typename T, typename A>
    std::optional<T> FindAndParse(std::string_view k, A &&parseParam) const;

    /**
     * 查找指定路径中的值，并parse为数值类型
     * 
     * @param k 查找路径
     * 
     * 查找失败或parse失败时返回std::nullopt
     */
    template<typename T>
	std::optional<T> FindAndParse(std::string_view k) const;

    /**
     * @copydoc ConfigGroup::Find(const Str8&) const
     *
     * @exception ConfigNodeKeyNotFound 参数路径不存在时抛出
     */
    const ConfigNode &operator[](std::string_view k) const;

    /**
     * 返回作为ConfigGroup的this指针
     * 
     * @note 仅在从父类接口调用时有意义，免得到处写dynamic_cast
     */
    const ConfigGroup &AsGroup() const override;

    const ConfigGroup *TryAsGroup() const noexcept override { return this; }

    bool IsGroup() const noexcept override { return true; }

    std::string ToString() const override;
};

/**
 * @brief 配置参数数组
 */
class ConfigArray : public ConfigNode
{
    std::vector<const ConfigNode*> array_;
    std::string tag_;

public:

    ConfigArray(std::vector<const ConfigNode*> &&content, std::string tag);

    /**
     * 取得指定位置的元素
     * 
     * @param idx 从0开始计的元素下标
     * 
     * @return 数组中的第idx元素的指针，idx越界时返回nullptr
     */
    const ConfigNode *At(size_t idx) const;

    /**
     * 取得指定位置的元素
     * 
     * @param idx 从0开始计的元素下标
     * 
     * @warning idx越界会UB 
     */
    const ConfigNode &operator[](size_t idx) const;

    /** 取得数组元素数量 */
    size_t Size() const;

    /** 取得数组tag名字，若未给出则为空字符串 */
    const std::string &GetTag() const { return tag_; }

    /**
     * 返回作为ConfigArray的this指针
     *
     * @note 仅在从父类接口调用时有意义，免得到处写dynamic_cast
     */
    const ConfigArray &AsArray() const override;

    const ConfigArray *TryAsArray() const noexcept override { return this; }

    bool IsArray() const noexcept override { return true; }

    std::string ToString() const override;

    auto begin() const { return array_.begin(); }
    auto end()   const { return array_.end(); }
};

/**
 * @brief 配置参数值，以字符串形式呈现
 */
class ConfigValue : public ConfigNode
{
    std::string str_;

public:

    explicit ConfigValue(std::string str);

    /** 取得字符串内容 */
    const std::string &GetStr() const;

    /** 取得字符串内容 */
    const std::string &operator*() const;

    /**
     * 返回作为ConfigValue的this指针
     *
     * @note 仅在从父类接口调用时有意义，免得到处写dynamic_cast
     */
    const std::string &AsValue() const override;

    const std::string *TryAsValue() const noexcept override { return &str_; }

    bool IsValue() const noexcept override { return true; }

    std::string ToString() const override;
};

/**
 * @brief 配置文件
 * 
 * 配置文件中的内容以配置树的形式呈现，有三种类型的节点：
 * - 参数集合 一个从名字到其他节点的映射，整个配置文件就是一个全局参数集合。
 * - 参数数组 一个从下标到其他节点的映射。
 * - 参数值 一个字符串
 * 
 * 以下是一个简单的配置文件示例：
@code
# This is a comment
window =
{
    title = "AGZ Application";
    size = { w = 640; h = 480; }; # This is another comment
};
###
    This is a comment block
###
books = (
{
    title = "HaHa";
    author = "Zhang3";
},
{
    title = "XiXi";
    author = "Li4";
});
tagged_array = F(1, 2, 3);
@endcode
 * 整个文件内容构成一个匿名参数集合，其中：
 * - window这个名字被映射到另一个参数集合
 * - book这个名字被映射到一个列表，这个列表的tag为空，元素类型为别的参数集合。列表元素类型不一定要一致
 * - window.size中的x被映射到参数值“640”
 * - tagged_array被映射到一个列表，这个列表的tag为F，元素类型为value
 * 
 * 要访问某个特定的元素，可以通过对应的路径来完成。比如：
@code
assert(root["window.size.x"].Parse<int> == 640);
assert(root["window.title"] == "AGZ Application");
@endcode
 */
class Config : public Uncopiable
{
    ObjArena<> arena_;

    const ConfigGroup *global_ = nullptr;

public:

    /**
     * 从字符串中加载配置
     * 
     * @note 这一操作会清除之前加载的内容
     * 
     * @return 加载成功时返回true，否则表明配置文件中有语法错误
     */
    bool LoadFromMemory(std::string_view src);

    /**
     * 从文本文件中加载配置
     * 
     * @note 这一操作会清除之前加载的内容
     * 
     * @return 加载成功时返回true，否则表明文件读取失败或有语法错误
     */
    bool LoadFromFile(std::string_view filename);

    /** 是否包含可用的配置文件内容 */
    bool IsAvailable() const;

    /** 清除已加载的配置文件内容 */
    void Clear();

    /** 取得全局参数集合 */
    const ConfigGroup &Root() const;
};

} // namespace AGZ

#include "Config.inl"
