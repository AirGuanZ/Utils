#pragma once

#include "../Misc/Common.h"
#include "Vec3.h"
#include "Vec4.h"

AGZ_NS_BEG(AGZ::Math)

template<typename T>
using Color3 = Vec3<T>;

template<typename T>
using Color4 = Vec4<T>;

using Color3f = Color3<float>;
using Color3d = Color3<double>;

using Color4f = Color4<float>;
using Color4d = Color4<double>;

#define COLOR_CONST(N, R, G, B, A) \
    namespace Aux \
    { \
        struct Color_##N##_t \
        { \
            constexpr Color_##N##_t() = default; \
            constexpr operator Color3f() const \
                { return Color3f(float(R),  float(G),  float(B)); } \
            constexpr operator Color3d() const \
                { return Color3d(double(R), double(G), double(B)); } \
            constexpr operator Color4f() const \
                { return Color4f(float(R),  float(G), \
                                 float(B),  float(A)); } \
            constexpr operator Color4d() const \
                { return Color4d(double(R), double(G), \
                                 double(B), double(A)); } \
        }; \
    } \
    inline constexpr Aux::Color_##N##_t N

namespace COLOR
{
    COLOR_CONST(VOID,  0.0, 0.0, 0.0, 0.0);
    COLOR_CONST(BLACK, 0.0, 0.0, 0.0, 1.0);
    COLOR_CONST(WHITE, 1.0, 1.0, 1.0, 1.0);
    COLOR_CONST(RED,   1.0, 0.0, 0.0, 1.0);
    COLOR_CONST(GREEN, 0.0, 1.0, 0.0, 1.0);
    COLOR_CONST(BLUE,  0.0, 0.0, 1.0, 1.0);
}

#undef COLOR_CONST

AGZ_NS_END(AGZ::Math)
