#pragma once

#include <tuple>

#include "TypeOpr.h"

namespace AGZ
{

template<typename...Types>
class RefList
{
    std::tuple<Types&...> refs_;

public:

    explicit RefList(Types&...refs) noexcept
        : refs_{ refs... }
    {
        
    }

    template<typename T, typename = std::enable_if_t<(TypeOpr::FindInTypeList<T&, Types&...> >= 0)>>
    T &Get() const noexcept
    {
        return std::get<T&>(refs_);
    }

    template<typename T, typename = std::enable_if_t<(TypeOpr::FindInTypeList<T&, Types&...> < 0) &&
                                                     (TypeOpr::FindInTypeList<const T&, Types&...> >= 0)>>
    const T &Get() const noexcept
    {
        return std::get<const T&>(refs_);
    }

    template<typename...SubTypes>
    auto GetSubList() const noexcept
    {
        return RefList<std::remove_reference_t<decltype(Get<SubTypes>())>...>(Get<SubTypes>()...);
    }
};

} // namespace AGZ
