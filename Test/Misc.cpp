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
}
