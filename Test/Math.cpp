#include <algorithm>

#include <Time/Prelude.h>
#include <Math/Prelude.h>

#include "Catch.hpp"

using namespace AGZ::Math;
using namespace std;

TEST_CASE("Math")
{
    SECTION("Angle")
    {
        REQUIRE(ApproxEq(PI<Radd>().value, 3.1415926, 1e-5));
        REQUIRE(ApproxEq(PI<Degd>().value, 180.0, 1e-7));
        REQUIRE(ApproxEq(PI<double>(), 3.1415926, 1e-5));

        REQUIRE(ApproxEq(Sin(PI<Degd>()), 0.0, 1e-5));
        REQUIRE(ApproxEq(Cos(PI<Degd>()), -1.0, 1e-5));
        REQUIRE(ApproxEq(Sin(PI<Degd>() / 2.0), 1.0, 1e-5));

        REQUIRE(ApproxEq(Sin(PI<Degd>() / 2.0 + PI<Degd>() / 2.0),
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

    SECTION("Proj")
    {
        Mat4d proj = Mat4d::Perspective(Degd(60.0), 640.0 / 480.0, 0.1, 100.0);
        REQUIRE((proj * Vec4d(0.0, 0.0, 0.0, 1.0)).z < 0.0);
        REQUIRE(ApproxEq(Homogenize(proj * Vec4d(0.0, 10.0 / Sqrt(3.0), 10.0, 1.0)).y,
                         1.0, 1e-5));
        REQUIRE(ApproxEq(Homogenize(proj * Vec4d(10.0 / Sqrt(3.0) * 640.0 / 480.0, 0.0, 10.0, 1.0)).x,
                         1.0, 1e-5));
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
        REQUIRE(ApproxEq(Vec3f(1.0, 2.0, 3.0) * Vec3f(2.0, 3.0, 4.0),
                         Vec3f(2.0, 6.0, 12.0), 1e-5f));

        REQUIRE(ApproxEq(Vec4d(1.0, 2.0, 3.0, 4.0).abgr(), Vec4d(4.0, 3.0, 2.0, 1.0), 1e-10));
        REQUIRE(ApproxEq(Vec4d(1.0, 2.0, 3.0, 4.0).xxz(), Vec3d(1.0, 1.0, 3.0), 1e-10));
        REQUIRE(ApproxEq(Vec4d(1.0, 2.0, 3.0, 4.0).um(), Vec2d(1.0, 3.0), 1e-10));
    }

    SECTION("Color")
    {
        REQUIRE(ApproxEq((Color4f)COLOR::VOID, Color4f(0.f, 0.f, 0.f, 0.f), 1e-10f));
        REQUIRE(ApproxEq((Color4f)COLOR::RED, Color4f(1.f, 0.f, 0.f, 1.f), 1e-10f));
        REQUIRE(ApproxEq((Color4f)COLOR::GREEN, Color4f(0.f, 1.f, 0.f, 1.f), 1e-10f));

        REQUIRE(ApproxEq(Clamp(Color4d(-4.0, 8.0, 0.0, 1.0), 0.0, 1.0),
                         Color4d(0.f, 1.f, 0.f, 1.f), 1e-10));
        REQUIRE(ApproxEq(2.0 * Color3d(-4.0, 8.0, 0.0) + Color3d(8.0, -15.0, 1.0),
                         Color3d(0.f, 1.f, 1.f), 1e-7));
    }

    SECTION("f32x4")
    {
        f32x4 a(1.0f, 2.0f, 3.0f, 4.0f);
        f32x4 b(2.0f, 3.0f, 4.0f, 5.0f);
        REQUIRE(ApproxEq(a + b, f32x4(3.0, 5.0, 7.0, 9.0), 1e-7f));

        REQUIRE(ApproxEq(Sqrt(f32x4(4.0f, 3.0f, 2.0f, 1.0f) * F32X4::ONE).AsVec(),
                         Vec4f(4.0f, 3.0f, 2.0f, 1.0f).Map<float>(
                            [](float x) { return Sqrt(x); }),
                         1e-5f));
        
        AGZ::Time::Bench::
        Run("Nor version", 20, [](){
            vector<Vec4f> data(10000);
            for(auto &v : data)
                v = Vec4f(1.0f, 2.0f, 3.0f, 4.0f);
            for(int i = 0; i != 100; ++i)
            {
                for(auto &v : data)
                    v = Sqrt(v);
            }
        }).
        Run("SSE version", 20, [](){
            vector<Vec4f> data(10000);
            for(auto &v : data)
                v = Vec4f(1.0f, 2.0f, 3.0f, 4.0f);
            for(int i = 0; i != 100; ++i)
            {
                for(auto &v : data)
                    v = Sqrt(f32x4(v)).AsVec();
            }
        });
    }
}
