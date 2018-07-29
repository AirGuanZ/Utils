#include <Utils.h>

#include <string>

#include "Catch.hpp"

using namespace AGZ;
using namespace std;

TEST_CASE("String")
{
    SECTION("UTF-8")
    {
        REQUIRE(UTF8<char>::Check("0123456789", 10));

        uint8_t data[4] = { 0, 1, 2, 255 };
        REQUIRE(!UTF8<uint8_t>::Check(data, 4));
    }

    SECTION("Constructor")
    {
        REQUIRE(Str(u8"minecraft", 9).Length() == 9);
        REQUIRE(Str(u8"ABCDEFGHIJKLMNOPQRSTUWVZYAABCDEFGHIJKLMNOPQRSTUWVZYA"
                      "ABCDEFGHIJKLMNOPQRSTUWVZYAABCDEFGHIJKLMNOPQRSTUWVZYA", 104).Length() == 104);
        REQUIRE(Str(u8"ABCDEFGHIJKLMNOPQRSTUWVZYAABCDEFGHIJKLMNOPQRSTUWVZYA"
                      "ABCDEFGHIJKLMNOPQRSTUWVZYAABCDEFGHIJKLMNOPQRSTUWVZYA", 104)[4] == u8'E');
        REQUIRE(Str(u8"今", 3).Length() == 3);

        string s = u8"今天minecraft天°气dark soul不错the witcher啊";
        auto sd = s.data();
        REQUIRE(Str(sd, sd + s.length()).Length() == s.length());
    }
}
