#include <Utils/Math.h>
#include <Utils/Texture.h>

#include "Catch.hpp"

using namespace AGZ::Math;
using namespace AGZ::Tex;

TEST_CASE("Texture")
{
    Texture<2, Color3f> tex0({640, 480});
    REQUIRE(tex0);
    REQUIRE(tex0.GetWidth() == 640);
    REQUIRE(tex0(5, 5) == Color3f());
}
