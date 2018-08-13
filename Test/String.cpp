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
        REQUIRE(Str8(u8"今", strlen(u8"今")).Length()        == 3);
        REQUIRE(Str8(u8"今天mine天气craft不错").ToStdString()  == u8"今天mine天气craft不错");
        REQUIRE(Str16(u8"今天mine天气craft不错").ToStdString() == u8"今天mine天气craft不错");
        REQUIRE(Str32(u8"今天mine天气craft不错").ToStdString() == u8"今天mine天气craft不错");

        {
            Str8 a = u8"Minecraft";
            Str8 b = a;
            REQUIRE(a == b);

            a = u8"minecraftminecraftminecraftminecraft";
            b = move(a);
            REQUIRE(a.Empty());
            REQUIRE(b == u8"minecraftminecraftminecraftminecraft");

            REQUIRE(Str8(b, 4, b.Length()) == u8"craftminecraftminecraftminecraft");
            REQUIRE(Str16(CSConv::Convert<UTF16<>>(b), 4, CSConv::Convert<UTF16<>>(b).Length())
                    == u8"craftminecraftminecraftminecraft");
        }
    }

    SECTION("From")
    {
        REQUIRE(Str8::From(10)                                            == u8"10");
        REQUIRE(Str8::From(-0)                                            == u8"0");
        REQUIRE(Str8::From(-10)                                           == u8"-10");
        REQUIRE(Str8::From(0)                                             == u8"0");
        REQUIRE(Str8::From(2, 2)                                          == u8"10");
        REQUIRE(Str8::From(0xFF35B, 16)                                   == u8"FF35B");
        REQUIRE(Str32::From(01234567u, 8)                                 == u8"1234567");
        REQUIRE(AStr::From(12 * 35 * 35 * 35 + 4 * 35 * 35 + 34 * 35, 35) == u8"C4Y0");
    }

    SECTION("Trim")
    {
        REQUIRE(Str8(u8"  Minecraft\n\t").Trim()      == u8"Minecraft");
        REQUIRE(Str8(u8"  Minecraft\n\t").TrimLeft()  == u8"Minecraft\n\t");
        REQUIRE(Str8(u8"  Minecraft\n\t").TrimRight() == u8"  Minecraft");
    }

    SECTION("Slice")
    {
        REQUIRE(Str8(u8"Minecraft").Slice(3)    == u8"ecraft");
        REQUIRE(Str8(u8"Minecraft").Slice(0, 3) == u8"Min");
        REQUIRE(Str8(u8"Minecraft").Slice(0, 6) == u8"Minecr");
    }

    SECTION("Split")
    {
        REQUIRE((Str8(u8"Mine cr aft ").Split()
                | Map([](const Str8::View &v) { return v.AsString(); })
                | Collect<vector<Str8>>())
             == vector<Str8>{ u8"Mine", u8"cr", u8"aft" });
        REQUIRE((Str8(u8"Minecreaft").Split(u8"e")
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
        REQUIRE(Str8(u8"Minecraft").Find(u8"Mine")      == 0);
        REQUIRE(Str8(u8"Minecraft").Find(u8"necraft")   == 2);
        REQUIRE(Str8(u8"Minecraft").Find(u8"Minecraft") == 0);
        REQUIRE(Str8(u8"Minecraft").Find(u8"eecraft")   == Str8::NPOS);
        REQUIRE(Str8(u8"Minecraft").Find(u8"er")        == Str8::NPOS);
    }

    /*SECTION("Regex")
    {
        REQUIRE(Regex<UTF8<>>(u8"[ef]c?").Match(u8"fc").Valid());
        REQUIRE(Regex<UTF8<>>(u8"abcd[ef]c?").Match(u8"abcdgc").Valid() == false);


        REQUIRE(Regex<UTF8<>>(u8"(def+)").Match(u8"defff").Valid());
        REQUIRE(Regex<UTF8<>>(u8"abc?[def]+").Match(u8"abdefdeffdef").Valid());

        {
            auto m = Regex<UTF8<>>(u8"abc?$([def]+)").Match(u8"abdefdeffdef");
            REQUIRE((m.Valid() && m[0] == u8"defdeffdef"));
        }

        REQUIRE_THROWS(Regex<UTF8<>>(u8"$($a)bc*").Match(u8"abdefdeffdef"));
    }*/
}
