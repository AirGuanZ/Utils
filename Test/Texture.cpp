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

    BinaryMemorySerializer serializer;
    serializer.Serialize(tex1);

    BinaryMemoryDeserializer deserializer(serializer.GetData(), serializer.GetByteSize());
    Texture2D<Color3f> tex2;
    deserializer.Deserialize(tex2);

    REQUIRE(tex2.GetWidth() == 640);
    REQUIRE(tex2.GetHeight() == 480);
    REQUIRE(ApproxEq(tex2(5, 5).x, 0.5f, 1e-4f));
}

TEST_CASE("SphereMapper")
{
    using S = SphereMapper<double>;

    REQUIRE(ApproxEq(S::InvMap(S::Map(Vec3d(1.0, 2.0, 3.0))), Vec3d(1.0, 2.0, 3.0).Normalize(), 1e-4));
}
