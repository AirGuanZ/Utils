#include <Utils.h>

#include <cstring>

#include "Catch.hpp"

using namespace AGZ;
using namespace std;

TEST_CASE("String")
{
    SECTION("Constructor")
    {
        REQUIRE(Str8().Length() == 0);
        REQUIRE(Str8(u8"minecraftminecraftminecraftminecraft", 36).Length() == 36);
        REQUIRE(Str8(u8"今", std::strlen(u8"今")).Length() == 3);
        REQUIRE(Str8(u8"今天mine天气craft不错").ToStdString() == u8"今天mine天气craft不错");
        REQUIRE(Str16(u8"今天mine天气craft不错").ToStdString() == u8"今天mine天气craft不错");
        REQUIRE(Str32(u8"今天mine天气craft不错").ToStdString() == u8"今天mine天气craft不错");

        {
            Str8 a = u8"Minecraft";
            Str8 b = a;
            REQUIRE(a == b);

            a = u8"minecraftminecraftminecraftminecraft";
            b = std::move(a);
            REQUIRE(a.Empty());
            REQUIRE(b == u8"minecraftminecraftminecraftminecraft");

            REQUIRE(Str8(b, 4, b.Length()) == u8"craftminecraftminecraftminecraft");
            REQUIRE(Str16(CSConv::Convert<UTF16<>>(b), 4, CSConv::Convert<UTF16<>>(b).Length())
                    == u8"craftminecraftminecraftminecraft");
        }
    }

    SECTION("Trim")
    {
        REQUIRE(Str8(u8"  Minecraft\n\t").Trim() == Str8(u8"Minecraft"));
        REQUIRE(Str8(u8"  Minecraft\n\t").TrimLeft() == Str8(u8"Minecraft\n\t"));
        REQUIRE(Str8(u8"  Minecraft\n\t").TrimRight() == Str8(u8"  Minecraft"));
    }

    SECTION("Split")
    {
        REQUIRE((Str8(u8"Mine cr aft ").Split()
                | Map([](const Str8::View &v) { return v.AsString(); })
                | Collect<vector<Str8>>())
             == vector<Str8>{ u8"Mine", u8"cr", u8"aft" });
        REQUIRE((Str8(u8"Minecreaft").Split(Str8(u8"e").AsView())
                | Map([](const Str8::View &v) { return v.AsString(); })
                | Collect<vector<Str8>>())
             == vector<Str8>{ u8"Min", u8"cr", u8"aft" });
    }

    SECTION("Join")
    {
        REQUIRE(Str8(" + ").Join(vector<Str8>{ u8"a", u8"b", u8"c" }) == u8"a + b + c");
        REQUIRE(Str8(" + ").Join(vector<Str8>{ }).Empty());
    }

    SECTION("Find")
    {
        REQUIRE(Str8(u8"Minecraft").Find(u8"Mine") == 0);
        REQUIRE(Str8(u8"Minecraft").Find(u8"necraft") == 2);
        REQUIRE(Str8(u8"Minecraft").Find(u8"Minecraft") == 0);
        REQUIRE(Str8(u8"Minecraft").Find(u8"eecraft") == Str8::NPOS);
        REQUIRE(Str8(u8"Minecraft").Find(u8"er") == Str8::NPOS);
    }
}
