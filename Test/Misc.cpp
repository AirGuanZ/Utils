#include <type_traits>
#include <AGZUtils/Utils/Misc.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace Endian;
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

    SECTION("Endian")
    {
        REQUIRE(Big2Little(uint8_t(0))   == uint8_t(0));
        REQUIRE(Big2Little(uint8_t(179)) == uint8_t(179));
        REQUIRE(Big2Little(uint8_t(255)) == uint8_t(255));

        REQUIRE(Big2Little(uint16_t(0xfe18))             != uint16_t(0xfe18));
        REQUIRE(Little2Big(Big2Little(uint16_t(0xfe18))) == uint16_t(0xfe18));

        REQUIRE(Little2Big(uint16_t(0xfe18))             != uint16_t(0xfe18));
        REQUIRE(Big2Little(Little2Big(uint16_t(0xfe18))) == uint16_t(0xfe18));

        REQUIRE(Little2Big(uint32_t(0x12345678))             != uint32_t(0x12345678));
        REQUIRE(Big2Little(Little2Big(uint32_t(0x12345678))) == uint32_t(0x12345678));

        if constexpr(IS_LITTLE_ENDIAN)
        {
            REQUIRE(Native2Little(uint32_t(0x12345678)) == uint32_t(0x12345678));
            REQUIRE(Native2Big(uint32_t(0x78563412))    == uint32_t(0x12345678));
        }
        else
        {
            REQUIRE(Native2Big(uint32_t(0x12345678))    == uint32_t(0x12345678));
            REQUIRE(Native2Little(uint32_t(0x78563412)) == uint32_t(0x12345678));
        }
    }
    
    SECTION("TypeOpr")
    {
        static_assert(Any_v<std::is_integral, float, double, int>);
        static_assert(!Any_v<std::is_class, int, short, unsigned char>);

        Variant<int, float, std::string> tu = 5;

        auto v = MatchVar(tu,
            [](float f) { return f; },
            [](int x) { return x + 2.0f; },
            [](auto) { return 0.0f; });
        REQUIRE(v == 7);
    }

    SECTION("PrettyTypeName")
    {
        REQUIRE(PrettyTypeName<int(*)(float, double)>() == "ptr-to (float, double) -> int");
        const int &x = 4;
        REQUIRE(PrettyTypeName<decltype((x))>() == "lref-to const int");
        REQUIRE(PrettyTypeName<const int * const>() == "const ptr-to const int");
    }
}
