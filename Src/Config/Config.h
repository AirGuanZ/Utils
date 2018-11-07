#pragma once

#include <vector>
#include <unordered_map>

#include "../Alloc/ObjArena.h"
#include "../Misc/Exception.h"
#include "../Misc/Uncopiable.h"
#include "../Utils/String.h"

namespace AGZ {

class ConfigGroup;
class ConfigArray;
class ConfigValue;

AGZ_NEW_EXCEPTION(ConfigNodeKeyNotFound);
AGZ_NEW_EXCEPTION(ConfigNodeInvalidCasting);

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
    virtual const Str8 &AsValue() const { throw ConfigNodeInvalidCasting("ConfigASTNode: invalid casting"); }

	//! 是否是Group类型
	virtual bool IsGroup() const { return false; }
	//! 是否是Array类型
	virtual bool IsArray() const { return false; }
	//! 是否是Value类型
	virtual bool IsValue() const { return false; }
};

/**
 * @brief 配置参数集合
 */
class ConfigGroup : public ConfigNode
{
    std::unordered_map<Str8, const ConfigNode*> children_;

    const ConfigNode *FindSection(const StrView8 &k) const;

public:

    explicit ConfigGroup(std::unordered_map<Str8, const ConfigNode*> &&children);

	/**
	 * 查找具有指定路径的配置参数值
	 * 
	 * @param k 带查找的参数路径，用“.”作为路径分隔符
	 */
    const ConfigNode *Find(const Str8 &k) const;
	//! @copydoc ConfigGroup::Find(const Str8&) const
    const ConfigNode *Find(const StrView8 &k) const;
	/**
	 * @copydoc ConfigGroup::Find(const Str8&) const
	 *
	 * @exception ConfigNodeKeyNotFound 参数路径不存在时抛出
	 */
    const ConfigNode &operator[](const Str8 &k) const;
	//! @copydoc ConfigGroup::operator[](const Str8 &k) const
    const ConfigNode &operator[](const StrView8 &k) const;

	/**
	 * 返回作为ConfigGroup的this指针
	 * 
	 * @note 仅在从父类接口调用时有意义，免得到处写dynamic_cast
	 */
    const ConfigGroup &AsGroup() const override;

	bool IsGroup() const override { return true; }
};

/**
 * @brief 配置参数数组
 */
class ConfigArray : public ConfigNode
{
    std::vector<const ConfigNode*> array_;

public:

    explicit ConfigArray(std::vector<const ConfigNode*> &&content);

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

	/**
	 * 返回作为ConfigArray的this指针
	 *
	 * @note 仅在从父类接口调用时有意义，免得到处写dynamic_cast
	 */
    const ConfigArray &AsArray() const override;

	bool IsArray() const override { return true; }
};

/**
 * @brief 配置参数值，以字符串形式呈现
 */
class ConfigValue : public ConfigNode
{
    Str8 str_;

public:

    explicit ConfigValue(Str8 &&str);

	/** 取得字符串内容 */
    const Str8 &GetStr() const;

	/** 取得字符串内容 */
    const Str8 &operator*() const;

	/**
	 * 返回作为ConfigValue的this指针
	 *
	 * @note 仅在从父类接口调用时有意义，免得到处写dynamic_cast
	 */
    const Str8 &AsValue() const override;

	bool IsValue() const override { return true; }
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
window =
{
	title = "AGZ Application";
	size = { w = 640; h = 480; };
};
books = (
{
	title = "HaHa";
	author = "Zhang3";
},
{
	title = "XiXi";
	author = "Li4";
});
@endcode
 * 整个文件内容构成一个匿名参数集合，其中：
 * - window这个名字被映射到另一个参数集合
 * - book这个名字被映射到一个列表，而列表元素的类型是参数集合
 * - window.size中的x被映射到参数值“640”
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
    bool LoadFromMemory(const Str8 &src);

	/**
	 * 从文本文件中加载配置
	 * 
	 * @note 这一操作会清除之前加载的内容
	 * 
	 * @return 加载成功时返回true，否则表明文件读取失败或有语法错误
	 */
    bool LoadFromFile(const Str8 &filename);

	/** 是否包含可用的配置文件内容 */
    bool IsAvailable() const;

	/** 清除已加载的配置文件内容 */
    void Clear();

	/** 取得全局参数集合 */
    const ConfigGroup &Root() const;
};

} // namespace AGZ

#include "Config.inl"
