#include <type_traits>
#include <AGZUtils/Utils/Misc.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace TypeOpr;

TEST_CASE("Misc")
{
    SECTION("Either")
    {
        Either<int, float> intOrFloat = 4;
        REQUIRE(intOrFloat.IsLeft());
        REQUIRE(!intOrFloat.IsRight());
        REQUIRE(intOrFloat.GetLeft() == 4);
        REQUIRE_THROWS(intOrFloat.GetRight());
    }
    
    SECTION("TypeOpr")
    {
        static_assert(Any_v<std::is_integral, float, double, int>);
        static_assert(!Any_v<std::is_class, int, short, unsigned char>);

        std::variant<int, float, std::string> tu = 5;

        auto v = MatchVariant(tu,
            [](float f) { return f; },
            [](int x) { return x + 2.0f; },
            [](auto) { return 0.0f; });
        REQUIRE(v == 7);
    }

    SECTION("RefList")
    {
        int a = 2;
        unsigned char b = 3;

        RefList<int, const unsigned char> L0(a, b);

        L0.Get<int>() = 5;
        REQUIRE(a == 5);
        REQUIRE(L0.Get<const unsigned char>() == 3);

        static_assert(std::is_same_v<decltype(L0.Get<unsigned char>()), const unsigned char&>);

        auto L1 = L0.GetSubList<unsigned char>();
        static_assert(std::is_same_v<decltype(L1), RefList<const unsigned char>>);

        auto L2 = L0.GetSubList<int>();
        L2.Get<int>() = 10;
        REQUIRE(a == 10);
    }
}
