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
        REQUIRE(Str8(u8"minecraft", 9).Length() == 9);
        REQUIRE(Str8(u8"ABCDEFGHIJKLMNOPQRSTUWVZYAABCDEFGHIJKLMNOPQRSTUWVZYA"
                       "ABCDEFGHIJKLMNOPQRSTUWVZYAABCDEFGHIJKLMNOPQRSTUWVZYA", 104).Length() == 104);
        REQUIRE(Str8(u8"ABCDEFGHIJKLMNOPQRSTUWVZYAABCDEFGHIJKLMNOPQRSTUWVZYA"
                       "ABCDEFGHIJKLMNOPQRSTUWVZYAABCDEFGHIJKLMNOPQRSTUWVZYA", 104)[4] == u8'E');
        REQUIRE(Str8(Str8(u8"今", 3)).Length() == 3);

        string s = u8"今天minecraft天°气dark souls不错the witcher啊";
        REQUIRE(Str8(s).Length() == s.length());
        REQUIRE(Str8(String<UTF8<uint32_t>>(Str32(Str8(s)))).Length() == s.length());
        REQUIRE(Str8(Str32(Str8(s))).Length() == s.length());

        REQUIRE(Str32(s).ToStdString() == s);
        REQUIRE(Str8(s).ToStdString() == s);
    }
}
