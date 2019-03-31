#pragma once

#include <map>
#include <memory>
#include <type_traits>
#include <unordered_map>

namespace AGZ
{
    
template<typename TK, typename TV, typename TV2K, bool TUseUnorderedMap = false>
class SharedPtrPool
{
public:

    using This = SharedPtrPool<TK, TV, TV2K, TUseUnorderedMap>;
    using VPtr = std::shared_ptr<TV>;

    SharedPtrPool()                                    = default;
    SharedPtrPool(const SharedPtrPool&)                = default;
    SharedPtrPool(SharedPtrPool&&) noexcept            = default;
    SharedPtrPool &operator=(const SharedPtrPool&)     = default;
    SharedPtrPool &operator=(SharedPtrPool&&) noexcept = default;

    VPtr Find(const TK &k) const
    {
        auto it = map_.find(k);
        return it == map_.end() ? it->second : nullptr;
    }

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

    void Erase(const TK &k)
    {
        auto p = Find(k);
        map_.erase(k);
    }

    void Erase(VPtr ptr)
    {
        Erase(TV2K()(*ptr));
    }

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
