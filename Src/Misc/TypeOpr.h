#pragma once

#include <utility>

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

BINARY_OPR(BoolAnd, &&);
BINARY_OPR(BoolOr, ||);

BINARY_OPR(BitAnd, &);
BINARY_OPR(BitOr, |);
BINARY_OPR(BitXor, ^);

UNARY_OPR(Pos, +);
UNARY_OPR(Neg, -);
UNARY_OPR(BoolNot, !);
UNARY_OPR(BitNot, ~);
UNARY_OPR(Deref, *);
UNARY_OPR(Addr, &);

#undef DV
#undef BINARY_OPR
#undef UNARY_OPR

AGZ_NS_END(AGZ::TypeOpr)
