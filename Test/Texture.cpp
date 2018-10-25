#include <Utils/Math.h>
#include <Utils/Texture.h>

#include "Catch.hpp"

using namespace AGZ::Math;
using namespace AGZ;

TEST_CASE("Texture")
{
    TextureCore<2, Color3f> tex0({ 640, 480 });
    REQUIRE(tex0.GetSize()[0] == 640);
    REQUIRE(tex0({ 5, 5 }) == Color3f());

    tex0.Clear(COLOR::RED);
    auto tex1 = tex0.Map([](const Color3f &c) { return c / 2.0f; });
    REQUIRE(ApproxEq(Float(tex1({ 5, 5 }).x), Float(0.5f)));
}

TEST_CASE("Texture2D")
{
    Texture2D<Color3f> tex0(640, 480);
    REQUIRE(tex0.GetSize()[0] == 640);
    REQUIRE(tex0(5, 5) == Color3f());

    tex0.Clear(COLOR::RED);
    auto tex1 = tex0.Map([](const Color3f &c) { return c / 2.0f; });
    REQUIRE(ApproxEq(Float(tex1(5, 5).x), Float(0.5f)));
}
