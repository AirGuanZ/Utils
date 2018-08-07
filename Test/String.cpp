#include <Utils.h>

#include <string>
#include <vector>

#include "Catch.hpp"

using namespace AGZ;
using namespace std;

TEST_CASE("UTF")
{
    SECTION("UTF-8")
    {
        
    }
}

/*
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
        {
            REQUIRE(Str8(u8"minecraft", 9).Length() == 9);
            REQUIRE(Str8(u8"ABCDEFGHIJKLMNOPQRSTUWVZYAABCDEFGHIJKLMNOPQRSTUWVZYA"
                "ABCDEFGHIJKLMNOPQRSTUWVZYAABCDEFGHIJKLMNOPQRSTUWVZYA", 104).Length() == 104);
            REQUIRE(Str8(u8"ABCDEFGHIJKLMNOPQRSTUWVZYAABCDEFGHIJKLMNOPQRSTUWVZYA"
                "ABCDEFGHIJKLMNOPQRSTUWVZYAABCDEFGHIJKLMNOPQRSTUWVZYA", 104)[4] == u8'E');
            REQUIRE(Str8(Str8(u8"今", 3)).Length() == 3);
        }

        {
            string s = u8"今天minecraft天°气dark souls不错the witcher啊";
            REQUIRE(Str8(s).Length() == s.length());
            REQUIRE(Str8(String<UTF8<uint32_t>>(Str32(Str32(s)))).Length() == s.length());
            REQUIRE(Str8(Str32(Str8(s))).Length() == s.length());

            Str8 a = s;
            REQUIRE(a.ToStdString() == s);
            REQUIRE(Str32(s).ToStdString() == s);

            REQUIRE(Str8(std::move(a)).ToStdString() == s);
            Str8 b = std::move(a);
            REQUIRE(b.IsEmpty());

            REQUIRE(Str8(s.data(), s.data() + s.length(), 5).ToStdString() == s + s + s + s + s);
        }

        {
            const char *s1 = u8"Minecraft", *s2 = u8" is a good game";
            REQUIRE(Str8(NOCHECK, s1, s1 + strlen(s1), s2, s2 + strlen(s2))
                == u8"Minecraft is a good game");
            REQUIRE(Str8(s1, s1 + strlen(s1), s2, s2 + strlen(s2))
                == u8"Minecraft is a good game");
        }

        {
            REQUIRE(Str32(u8"Minecraft is a good game") == u8"Minecraft is a good game");
            REQUIRE(Str8(u8"今天天气真不错") == u8"今天天气真不错");
            REQUIRE(Str16(u8"今天天气真不错") == u8"今天天气真不错");
            REQUIRE(Str32(u8"今天天气真不错") == u8"今天天气真不错");
            REQUIRE(Str32(u8"今天天气真不错。") != u8"今天天气真不错");
        }
    }

    SECTION("Swap & Exchange")
    {
        Str8 s1(u8"Minecraft os a good game"), s2(u8"Hahaha，今天°天气真不错");
        s1.Swap(s2);
        REQUIRE(s1 == u8"Hahaha，今天°天气真不错");
        REQUIRE(s2 == u8"Minecraft os a good game");

        Str16 s3(s1.Exchange(Str8(u8"Good morning~")));
        REQUIRE(s3 == u8"Hahaha，今天°天气真不错");
        REQUIRE(s1 == u8"Good morning~");
    }

    SECTION("Chars")
    {
        (Str8(u8"今天天气不错").Chars() | Collect<vector<char32_t>>()) ==
        (Str16(u8"今天天气不错").Chars() | Collect<vector<char32_t>>());


        (Str8(u8"今天天气不错，今天天气不错，今天天气不错。").Chars() |
            Collect<vector<char32_t>>()) ==
        (Str32(u8"今天天气不错，今天天气不错，今天天气不错。").Chars() |
            Collect<vector<char32_t>>());
    }

    SECTION("Join")
    {
        REQUIRE((vector<Str8>{ u8"mine", u8"craft" } | Join(u8"HM")) == u8"mineHMcraft");
        REQUIRE((vector<Str8>{ u8"a", u8"b", u8"c" } | Join(u8" + ")) == u8"a + b + c");
        REQUIRE((vector<Str8>{ } | Join(u8" + ", u8"Nothing!")) == u8"Nothing!");
    }
}
*/
