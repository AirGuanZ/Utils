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

        REQUIRE(Str8('a', 7) == "aaaaaaa");
        REQUIRE(Str8(0x4ECA, 5) == u8"今今今今今");
        REQUIRE(Str32(0x4ECA, 5) == u8"今今今今今");

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
        REQUIRE(Str16::From(0xFF35B, 16)                                  == u8"FF35B");
        REQUIRE(Str32::From(01234567u, 8)                                 == u8"1234567");
        REQUIRE(AStr::From(12 * 35 * 35 * 35 + 4 * 35 * 35 + 34 * 35, 35) == u8"C4Y0");
    }

    SECTION("Parse")
    {
        REQUIRE(Str8("123456").Parse<int>()    == 123456);
        REQUIRE(Str16("123abc").Parse<int>(16) == 0x123ABC);
        REQUIRE(Str16("C4Y0").Parse<int>(35)   == 12 * 35 * 35 * 35 + 4 * 35 * 35 + 34 * 35);
        REQUIRE(Str16("-123456").Parse<int>()  == -123456);
        REQUIRE(Str16("-0").Parse<int>()       == 0);

        REQUIRE(Math::ApproxEq(Str16("3.286").Parse<float>(), 3.286f, 1e-5));
    }

    SECTION("Misc")
    {
        REQUIRE(Str8(u8"  Minecraft\n\t").Trim()      == u8"Minecraft");
        REQUIRE(Str8(u8"  Minecraft\n\t").TrimLeft()  == u8"Minecraft\n\t");
        REQUIRE(Str8(u8"  Minecraft\n\t").TrimRight() == u8"  Minecraft");

        REQUIRE(Str8(u8"Minecraft").Slice(3) == u8"ecraft");
        REQUIRE(Str8(u8"Minecraft").Slice(0, 3) == u8"Min");
        REQUIRE(Str8(u8"Minecraft").Slice(0, 6) == u8"Minecr");

        REQUIRE(Str8(u8"今天天气不错").Prefix(Str8(u8"今天").Length())       == u8"今天");
        REQUIRE(Str8(u8"今天天气不错abc").Suffix(Str8(u8"不错abc").Length()) == u8"不错abc");

        REQUIRE(Str8(u8"Minecraft").StartsWith(u8"Minecra"));
        REQUIRE(Str8(u8"Minecraft").EndsWith(u8"necraft"));
        REQUIRE(Str8(u8"Minecraft").EndsWith(u8"Minecraft"));
        REQUIRE(!Str8(u8"Minecraft").EndsWith(u8"Minecra"));

        REQUIRE(Str8(u8"Z").IsDigit(36));
        REQUIRE(!Str8(u8"0Z").IsDigit(36));
        REQUIRE(!Str16(u8"仅").IsDigit(36));
        REQUIRE(Str8(u8"0123456").IsDigits());
        REQUIRE(!Str8(u8"012a3456").IsDigits());
        REQUIRE(Str8(u8"012a3456").IsDigits(16));

        REQUIRE(!Str8(u8"abcdefABCDEF").IsAlpha());
        REQUIRE(!Str8(u8"。").IsAlpha());
        REQUIRE(Str8(u8"X").IsAlphas());
        REQUIRE(Str8(u8"abcdefABCDEF").IsAlphas());
        REQUIRE(!Str8(u8"abcde!fABCDEF").IsAlphas());

        REQUIRE(Str8(u8"MINE").IsUppers());
        REQUIRE(Str8(u8"mine").IsLowers());
        REQUIRE(!Str8(u8"mine").IsUppers());
        REQUIRE(!Str8(u8"MINE").IsLowers());

        REQUIRE(Str8(u8"ABC").ToLower()   == u8"abc");
        REQUIRE(Str8(u8"Ab仅C").ToLower() == u8"ab仅c");
        REQUIRE(Str8(u8"abc").ToUpper()   == u8"ABC");
        REQUIRE(Str8(u8"Ab仅c").ToUpper() == u8"AB仅C");
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

    SECTION("Chars")
    {
        REQUIRE((Str8(u8"abc").Chars() | Collect<vector<Str8>>())
            == vector<Str8>{ u8"a", u8"b", u8"c" });
        REQUIRE((Str8(u8"今天a天气!").Chars() | Collect<vector<Str8>>())
            == vector<Str8>{ u8"今", u8"天", u8"a", u8"天", u8"气", u8"!" });
    }

    SECTION("Regex")
    {
        REQUIRE(Regex8(u8"abc").Match(u8"abc"));
        REQUIRE(!Regex8(u8"abc").Match(u8"ac"));
        REQUIRE(Regex8(u8"abc[def]").Match(u8"abcd"));
        REQUIRE(Regex8(u8"abc*").Match(u8"abccc"));
        REQUIRE(Regex8(u8"abc*").Match(u8"ab"));
        REQUIRE(!Regex8(u8"abc+").Match(u8"ab"));
        REQUIRE(Regex8(u8"abc?").Match(u8"ab"));
        REQUIRE(Regex8(u8"ab[def]+").Match(u8"abdefdeffeddef"));
        REQUIRE(Regex8(u8"今天天气不错").Search(u8"GoodMorning今天天气不错啊"));
        REQUIRE(Regex16(u8"今天(天气)+不错啊?\\?").Match(u8"今天天气天气天气天气不错?"));
        
        {
            auto m = Regex8(u8"&abc&(def)+&xyz&").Match(u8"abcdefdefxyz");
            REQUIRE((m && m(0, 1) == u8"abc" && m(1, 2) == u8"defdef" && m(2, 3) == u8"xyz"));
        }

        REQUIRE(Regex8(u8"b").Search(u8"abc"));
        REQUIRE(Regex16(u8"b+").Search(u8"abbbc"));

        {
            auto m = Regex8(u8"&b+&").Search(u8"abbbc");
            REQUIRE((m && m(0, 1) == u8"bbb"));
        }

        {
            auto m = Regex16(u8"&abcde&$").Search(u8"minecraftabcde");
            REQUIRE((m && m(0, 1) == u8"abcde"));
        }

        {
            auto m = Regex8("&[def]+&").Search("abcddeeffxyz");
            auto n = m;
            REQUIRE((m && m(0, 1) == "ddeeff"));
        }

        REQUIRE(Regex8(u8"mine").Search(u8"abcminecraft"));
        REQUIRE(Regex32(u8"^mine").Search(u8"abcminecraft") == false);
    }
}
