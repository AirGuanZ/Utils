#include "Catch.hpp"

#include <string>

#include <Math/Prelude.h>
#include <Result/Prelude.h>

using namespace AGZ;
using namespace Math;
using namespace std;

Option<float> GetSqrt(float v)
{
    if(v >= 0.0)
        return Some(Sqrt(v));
    return None<float>();
}

Option<string> GetSomeStr(const string &s)
{
    static_assert(sizeof(string) > 8);
    return Some<string>(s);
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

        REQUIRE(GetSomeStr("Minecraft is a good game!").Unwrap()
                        == "Minecraft is a good game!");
    }
}
