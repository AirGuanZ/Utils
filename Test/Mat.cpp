#include <Math\Prelude.h>

#include "Catch.hpp"

using namespace AGZ::Math;

TEST_CASE("Mat4")
{
    SECTION("Deg & Rad")
    {
        REQUIRE(ApproxEq(PI<Rad<double>>().value, 3.1415926, 1e-3));
        REQUIRE(ApproxEq(PI<Deg<double>>().value, 180.0, 1e-7));

        REQUIRE(ApproxEq(Sin(PI<Deg<double>>()), 0.0, 1e-5));
        REQUIRE(ApproxEq(Cos(PI<Deg<double>>()), -1.0, 1e-5));
        REQUIRE(ApproxEq(Sin(PI<Deg<double>>() / 2.0), 1.0, 1e-5));

        REQUIRE(ApproxEq(Sin(PI<Deg<double>>() / 2.0
                           + PI<Deg<double>>() / 2.0),
                         0.0, 1e-5));
    }

    SECTION("Transform")
    {
        Vec4d v(1.0, 2.0, 3.0, 1.0);

        REQUIRE(ApproxEq(Mat4d::Translate({ 3.0, 2.0, 1.0 }) * v,
                         Vec4d(4.0, 4.0, 4.0, 1.0), 1e-5));
    }
}
