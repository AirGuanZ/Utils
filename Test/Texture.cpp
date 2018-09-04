#include <Utils/Math.h>
#include <Utils/Texture.h>

#include "Catch.hpp"

using namespace AGZ::Math;
using namespace AGZ::Tex;

TEST_CASE("Texture2D")
{
    SECTION("Basic")
    {
        Texture2D<Color3f> tex0(640, 480);
        REQUIRE(tex0);
        REQUIRE((tex0.GetWidth() == 640 && tex0.GetHeight() == 480));
        REQUIRE(tex0(5, 5) == Color3f());
    }
}
