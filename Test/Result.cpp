#include "Catch.hpp"

#include <Math/Prelude.h>
#include <Result/Prelude.h>

using namespace AGZ;
using namespace Math;

Option<float> GetSqrt(float v)
{
    if(v >= 0.0)
        return Some(Sqrt(v));
    return None<float>();
}

TEST_CASE("Result")
{
    SECTION("Option")
    {
        REQUIRE(GetSqrt(1.0f).IsSome());
        REQUIRE(GetSqrt(-1.0f).IsNone());

        auto s = GetSqrt(5.0f);
        auto g = s;
        s = std::move(g);
        REQUIRE(s.IsSome());

        REQUIRE(ApproxEq(GetSqrt(2.0f).Unwrap(), Sqrt(2.0f), 1e-5f));
    }
}
