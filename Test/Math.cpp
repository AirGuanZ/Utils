#include <iostream>

#include <Math/Prelude.h>

#include "Catch.hpp"

using namespace AGZ::Math;
using namespace std;

TEST_CASE("Mat4")
{
    SECTION("Angle")
    {
        REQUIRE(ApproxEq(PI<Radd>().value, 3.1415926, 1e-5));
        REQUIRE(ApproxEq(PI<Degd>().value, 180.0, 1e-7));
        REQUIRE(ApproxEq(PI<double>(), 3.1415926, 1e-5));

        REQUIRE(ApproxEq(Sin(PI<Degd>()), 0.0, 1e-5));
        REQUIRE(ApproxEq(Cos(PI<Degd>()), -1.0, 1e-5));
        REQUIRE(ApproxEq(Sin(PI<Degd>() / 2.0), 1.0, 1e-5));

        REQUIRE(ApproxEq(Sin(PI<Degd>() / 2.0
                           + PI<Degd>() / 2.0),
                         0.0, 1e-5));
    }

    SECTION("Transform")
    {
        Vec4d v(1.0, 2.0, 3.0, 1.0);

        Mat4d m = Mat4d::Translate({ 3.0, 2.0, 1.0 });
        REQUIRE(ApproxEq(m * v, Vec4d(4.0, 4.0, 4.0, 1.0), 1e-5));
        REQUIRE(ApproxEq(Inverse(m) * m * v, v, 1e-5));

        m = Mat4d::Scale({ 1.0, 2.0, 3.0 });
        REQUIRE(ApproxEq(m * v, Vec4d(1.0, 4.0, 9.0, 1.0), 1e-5));
        REQUIRE(ApproxEq(Inverse(m) * m * v, v, 1e-5));

        v = Vec4d::UNIT_X();
        m = Mat4d::Rotate(Vec3d::UNIT_Z(), Degd(90.0));
        REQUIRE(ApproxEq(m * v, Vec4d::UNIT_Y(), 1e-5));
        REQUIRE(ApproxEq(Inverse(m) * m * v, v, 1e-5));

        v = Vec4d::UNIT_Y();
        m = Mat4d::Rotate(Vec3d::UNIT_X(), PI<Radd>() / 2.0);
        REQUIRE(ApproxEq(m * v, Vec4d::UNIT_Z(), 1e-5));
        REQUIRE(ApproxEq(Inverse(m) * m * v, v, 1e-5));

        v = Vec4d::UNIT_X();
        m = Mat4d::RotateZ(Degd(90.0));
        REQUIRE(ApproxEq(m * v, Vec4d::UNIT_Y(), 1e-5));
        REQUIRE(ApproxEq(Inverse(m) * m * v, v, 1e-5));

        v = Vec4d::UNIT_Y();
        m = Mat4d::RotateX(PI<Radd>() / 2.0);
        REQUIRE(ApproxEq(m * v, Vec4d::UNIT_Z(), 1e-5));
        REQUIRE(ApproxEq(Inverse(m) * m * v, v, 1e-5));
    }

    SECTION("Vec")
    {
        REQUIRE(ApproxEq(Cross(Vec3d::UNIT_X(), Vec3d::UNIT_Y()), Vec3d::UNIT_Z(), 1e-5));
        REQUIRE(ApproxEq(Cross(Vec3d::UNIT_Y(), Vec3d::UNIT_Z()), Vec3d::UNIT_X(), 1e-5));
        REQUIRE(ApproxEq(Cross(Vec3d::UNIT_Z(), Vec3d::UNIT_X()), Vec3d::UNIT_Y(), 1e-5));

        REQUIRE(ApproxEq(Dot(Vec4d::UNIT_X(), Vec4d::UNIT_Z()), 0.0, 1e-7));
        REQUIRE(ApproxEq(Dot(Mat4d::RotateY(Degd(90.0)) * Vec4d(1.0, 0.0, 4.0, 0.0),
                             Vec4d(1.0, 0.0, 4.0, 0.0)),
                         0.0, 1e-7));

        REQUIRE(ApproxEq(2.f * Vec2f(1.0, 2.0) + Vec2f(2.0, 3.0), Vec2f(4.0, 7.0), 1e-5f));
        REQUIRE(ApproxEq(Vec3f(1.0, 2.0, 3.0) * Vec3f(2.0, 3.0, 4.0), Vec3f(2.0, 6.0, 12.0), 1e-5f));
    }

    SECTION("Color")
    {
        REQUIRE(ApproxEq((Color4f)COLOR::RED, Color4f(1.f, 0.f, 0.f, 1.f), 1e-10f));
        REQUIRE(ApproxEq((Color4f)COLOR::GREEN, Color4f(0.f, 1.f, 0.f, 1.f), 1e-10f));

        REQUIRE(ApproxEq(Clamp(Color4d(-4.0, 8.0, 0.0, 1.0), 0.0, 1.0), Color4d(0.f, 1.f, 0.f, 1.f), 1e-10));
        REQUIRE(ApproxEq(2.0 * Color3d(-4.0, 8.0, 0.0) + Color3d(8.0, -15.0, 1.0), Color3d(0.f, 1.f, 1.f), 1e-7));
    }
}
