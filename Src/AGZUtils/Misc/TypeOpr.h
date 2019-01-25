#pragma once

#include <tuple>
#include <variant>

namespace AGZ::TypeOpr {

template<typename...TypeList>
constexpr size_t TypeListLength_v = std::tuple_size_v<std::tuple<TypeList...>>;

template<size_t Index, typename...TypeList>
using SelectInTypeList_t = std::tuple_element_t<Index, std::tuple<TypeList...>>;

template<template<typename> typename FuncClass>
constexpr bool Any() { return false; }

template<template<typename> typename FuncClass, typename T, typename...Others>
constexpr bool Any() { return FuncClass<T>::value || Any<FuncClass, Others...>(); }

template<template<typename> typename FuncClass, typename...TypeList>
constexpr bool Any_v = Any<FuncClass, TypeList...>();

template<template<typename> typename FuncClass>
constexpr bool All() { return true; }

template<template<typename> typename FuncClass, typename T, typename...Others>
constexpr bool All() { return FuncClass<T>::value && All<FuncClass, Others...>(); }

template<template<typename> typename FuncClass, typename...TypeList>
constexpr bool All_v = All<FuncClass, TypeList...>();

template<bool B> struct TrueToVoid { };
template<> struct TrueToVoid<true>  { using type = void; };

template<bool B>
using TrueToVoid_t = typename TrueToVoid<B>::type;

template<typename T>
constexpr bool True_v = true;

template<typename T>
using Void_t = void;

template<typename T>
struct CanConvertToGenerator
{
    template<typename U>
    using type = std::is_convertible<U, T>;
};

template<typename To, typename From>
To StaticCaster(const From &from)
{
    return static_cast<To>(from);
}

} // namespace AGZ::TypeOpr
