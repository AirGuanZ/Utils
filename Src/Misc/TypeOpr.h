#pragma once

#include <tuple>
#include <variant>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ::TypeOpr)

template<typename...TypeList>
constexpr size_t TypeListLength_v = std::tuple_size_v<std::tuple<TypeList...>>;

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

template<typename...Ts>
using Variant = std::variant<Ts...>;

template<typename E, typename...Vs>
auto MatchVar(E &&e, Vs...vs)
{
    struct overloaded : Vs...
    {
        explicit overloaded(Vs...vss)
            : Vs(vss)...
        {

        }
    };

    return std::visit(overloaded(vs...), std::forward<E>(e));
}

AGZ_NS_END(AGZ::TypeOpr)
