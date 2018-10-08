#pragma once

#include <tuple>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ::TypeOpr)

#define BINARY_OPR(Name, Opr) \
    template<typename L, typename R> \
    using Name = decltype(std::declval<L>() Opr std::declval<R>())
#define UNARY_OPR(Name, Opr) \
    template<typename T> \
    using Name = decltype(Opr std::declval<T>())

BINARY_OPR(Add, +);
BINARY_OPR(Sub, -);
BINARY_OPR(Mul, *);
BINARY_OPR(Div, /);
BINARY_OPR(Mod, %);

BINARY_OPR(Greater, >);
BINARY_OPR(GreaterEqual, >=);
BINARY_OPR(Equal, ==);
BINARY_OPR(NotEqual, !=);
BINARY_OPR(LessEqual, <=);
BINARY_OPR(Less, <);

BINARY_OPR(LogicAnd, &&);
BINARY_OPR(LogicOr, ||);

BINARY_OPR(BitAnd, &);
BINARY_OPR(BitOr, |);
BINARY_OPR(BitXor, ^);

UNARY_OPR(Pos, +);
UNARY_OPR(Neg, -);
UNARY_OPR(LogicNot, !);
UNARY_OPR(BitNot, ~);

template<typename T>
using Deref = decltype(*std::declval<T>());

template<typename T>
using Addr = T*;

template<typename F, typename...Args>
using Apply = decltype(std::declval<F>()(std::declval<Args>()...));

#undef DV
#undef BINARY_OPR
#undef UNARY_OPR

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

AGZ_NS_END(AGZ::TypeOpr)
