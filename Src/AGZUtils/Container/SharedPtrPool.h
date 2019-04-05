#pragma once

#include <map>
#include <memory>
#include <type_traits>
#include <unordered_map>

namespace AGZ
{
    
/**
 * @brief 共享数据池
 * 
 * 用于应对这样的需求：需要以shared_ptr的形式共享存储一些key相同的value，而当没有人持有共享数据时它应被自动删除
 * 也就是说，得到数据的步骤是：
 * 
 * 1. 在数据池中查看是否有key相同的共享数据可以用，若是，取共享数据；否则进入2
 * 2. 重新创建数据，并加入数据池
 * 
 * 而每当一个数据持有者放弃所有权时，都需要检查自己是否是最后一个数据持有人，若是，则应从数据池中删除该数据
 * 
 * @tparam TK 键类型
 * @tparam TV 数据类型
 * @tparam TV2K 从数据实例到键值的映射类，需使得 (bool)TV2K()(std::declval<TV&>()) 合法
 * @tparam TUseUnorderedMap 内部是使用unordered_map还是map
 * 
 * @note 线程不安全
 */
template<typename TK, typename TV, typename TV2K, bool TUseUnorderedMap = false>
class SharedPtrPool
{
public:

    using This = SharedPtrPool<TK, TV, TV2K, TUseUnorderedMap>;
    using VPtr = std::shared_ptr<TV>;

    SharedPtrPool()                  = default;
    SharedPtrPool(const This&)       = default;
    SharedPtrPool(This&&) noexcept   = default;
    This &operator=(const This&)     = default;
    This &operator=(This&&) noexcept = default;

    /**
     * @brief 在数据池中查找具有给定键值的数据，查找失败时返回nullptr
     */
    VPtr Find(const TK &k) const
    {
        auto it = map_.find(k);
        return it != map_.end() ? it->second : nullptr;
    }

    /**
     * @brief 尝试取得具有给定键值的数据，若数据池中不存在，则用所给的参数创建一个新的
     */
    template<typename...Args>
    VPtr GetOrNew(const TK &k, Args&&...args)
    {
        if(auto ret = Find(k))
            return ret;

        TV *data = new TV(std::forward<Args>(args)...);
        VPtr ret;
        try
        {
            ret = VPtr(data, Deleter{ this });
        }
        catch(...)
        {
            if(data)
                delete data;
            return nullptr;
        }

        map_[k] = ret;
        return ret;
    }

    /**
     * @brief 尝试取得具有给定键值的数据，若数据池中不存在，则调用addFunc()得到一个数据指针并加入数据池
     */
    template<typename AddFunc>
    VPtr GetOrAdd(const TK &k, AddFunc &&addFunc)
    {
        if(auto ret = Find(k))
            return ret;

        TV *data = addFunc();
        if(!data)
            return nullptr;

        VPtr ret;
        try
        {
            ret = VPtr(data, Deleter{ this });
        }
        catch(...)
        {
            if(data)
                delete data;
            return nullptr;
        }

        map_[k] = ret;
        return ret;
    }

    /**
     * @brief 从数据池中移除具有给定键值的数据
     */
    void Erase(const TK &k)
    {
        auto p = Find(k);
        map_.erase(k);
    }

    /**
     * @brief 从数据池中移除给定的数据
     */
    void Erase(VPtr ptr)
    {
        Erase(TV2K()(*ptr));
    }

    /**
     * @brief 清空数据池
     */
    void Clear()
    {
        while(!map_.empty())
            Erase(map_.begin()->first);
    }

    auto begin() { return map_.begin(); }
    auto end()   { return map_.end(); }
    auto begin() const { return map_.begin(); }
    auto end()   const { return map_.end(); }

private:

    using UMap = std::unordered_map<TK, VPtr>;
    using Map = std::map<TK, VPtr>;

    struct Deleter
    {
        This *pool;

        void operator()(TV *ptr) const noexcept
        {
            auto it = pool->map_.find(TV2K()(*ptr));
            if(it != pool->map_.end() && it->second.use_count() == 1 && it->second.get() == ptr)
                pool->map_.erase(it);
        }
    };

    std::conditional_t<TUseUnorderedMap, UMap, Map> map_;
};

} // namespace AGZ
