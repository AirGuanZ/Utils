#include <cstdint>

#include <Utils.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace Endian;
using namespace std;

TEST_CASE("Endian")
{
    REQUIRE(Big2Little(uint8_t(0)) == uint8_t(0));
    REQUIRE(Big2Little(uint8_t(179)) == uint8_t(179));
    REQUIRE(Big2Little(uint8_t(255)) == uint8_t(255));

    REQUIRE(Big2Little(uint16_t(0xfe18)) != uint16_t(0xfe18));
    REQUIRE(Little2Big(Big2Little(uint16_t(0xfe18))) == uint16_t(0xfe18));

    REQUIRE(Little2Big(uint16_t(0xfe18)) != uint16_t(0xfe18));
    REQUIRE(Big2Little(Little2Big(uint16_t(0xfe18))) == uint16_t(0xfe18));

    REQUIRE(Little2Big(uint32_t(0x12345678)) != uint32_t(0x12345678));
    REQUIRE(Big2Little(Little2Big(uint32_t(0x12345678))) == uint32_t(0x12345678));

    if constexpr(IS_LITTLE_ENDIAN)
    {
        REQUIRE(Native2Little(uint32_t(0x12345678)) == uint32_t(0x12345678));
        REQUIRE(Native2Big(uint32_t(0x78563412)) == uint32_t(0x12345678));
    }
    else
    {
        REQUIRE(Native2Big(uint32_t(0x12345678)) == uint32_t(0x12345678));
        REQUIRE(Native2Little(uint32_t(0x78563412)) == uint32_t(0x12345678));
    }
}
