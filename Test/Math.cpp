#include <iostream>

#include <Math/Prelude.h>

#include "Catch.hpp"
#include "Math/Random.h"

using namespace AGZ::Math;
using namespace std;

TEST_CASE("Mat4")
{
    SECTION("Deg & Rad")
    {
        REQUIRE(ApproxEq(PI<Radd>().value, 3.1415926, 1e-3));
        REQUIRE(ApproxEq(PI<Degd>().value, 180.0, 1e-7));

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
    }
}
