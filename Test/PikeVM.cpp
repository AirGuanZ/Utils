#include <Utils/String.h>

#include "Catch.hpp"

using namespace AGZ;

void P8(const WStr &str)
{
    (void)PikeVM::Parser<WUTF>().Parse(str);
}

void Gen(const WStr &str)
{
    uint32_t slotCount;
    (void)PikeVM::Backend<WUTF>().Generate(
        PikeVM::Parser<WUTF>().Parse(str), &slotCount);
}

TEST_CASE("VMEngEx")
{
    SECTION("Parser")
    {
        REQUIRE_NOTHROW(P8("minecraft"));
        REQUIRE_NOTHROW(P8("a|b|c|d|e"));
        REQUIRE_NOTHROW(P8("[abc]|[a-zA-Zdef0-9.\\\\..\\\\][m-x]"));
        REQUIRE_NOTHROW(P8("[abcA-Z]+*??*+"));
        REQUIRE_NOTHROW(P8("^abcde(abc$|[def]$)"));
        REQUIRE_NOTHROW(P8("abc&def&ghi"));
        REQUIRE_NOTHROW(P8("abc.\\.def.ghi\\..\\\\\\.."));
        REQUIRE_NOTHROW(P8("(abcdef)[xyz]{ 10 }"));
        REQUIRE_NOTHROW(P8("(abcdef)[xyz]{3, 18}"));
        REQUIRE_NOTHROW(P8("(abcdef)[xyz]{0, 18}"));
        REQUIRE_NOTHROW(P8("abc&@{![a-zA-Z]|([d-f]&(A|Z))}"));
        REQUIRE_NOTHROW(P8("abc\\d\\c\\w\\s\\h\\\\\\.\\\\\\...\\d"));
    }

    SECTION("Backend")
    {
        REQUIRE_NOTHROW(Gen("minecraft"));
        REQUIRE_NOTHROW(Gen("a|b|c|d|e"));
        REQUIRE_NOTHROW(Gen("[abc]|[a-zA-Zdef0-9.\\\\..\\\\][m-x]"));
        REQUIRE_NOTHROW(Gen("[abcA-Z]+*??*+"));
        REQUIRE_NOTHROW(Gen("^abcde(abc$|[def]$)"));
        REQUIRE_NOTHROW(Gen("abc&def&ghi"));
        REQUIRE_NOTHROW(Gen("abc.\\.def.ghi\\..\\\\\\.."));
        REQUIRE_NOTHROW(Gen("(abcdef)[xyz]{ 10 }"));
        REQUIRE_NOTHROW(Gen("(abcdef)[xyz]{3, 18}"));
        REQUIRE_NOTHROW(Gen("(abcdef)[xyz]{0, 18}"));
        REQUIRE_NOTHROW(Gen("a&@{![a-z]|([d-f]&(A|Z))}"));
        REQUIRE_NOTHROW(Gen("abc\\d\\c\\w\\s\\h\\\\\\.\\\\\\...\\d"));
    }

    SECTION("Machine")
    {
        REQUIRE(Regex8(u8"abc").Match(u8"abc"));
        REQUIRE(!Regex8(u8"abc").Match(u8"ac"));
        REQUIRE(Regex8(u8"abc[def]").Match(u8"abcd"));
        REQUIRE(Regex8(u8"abc*").Match(u8"abccc"));
        REQUIRE(Regex8(u8"abc*").Match(u8"ab"));
        REQUIRE(Regex8(u8"ab.\\.c+").Match(u8"abe.cc"));
        REQUIRE(Regex8(u8"abc?").Match(u8"ab"));
        REQUIRE(Regex8(u8"ab[def]+").Match(u8"abdefdeffeddef"));
        REQUIRE(Regex16(u8"今天(天气)+不错啊?\\?").Match(u8"今天天气天气天气天气不错?"));
        REQUIRE(Regex8(u8"今天天气不错").Search(u8"GoodMorning今天天气不错啊"));

        REQUIRE(Regex8(u8".*").Match(u8"今天天气不错啊"));
        REQUIRE(Regex8(u8"今天.*啊").Match(u8"今天天气不错啊"));

        REQUIRE(Regex8(u8"今天{ 5 \t }天气不错啊").Match(L"今天天天天天天气不错啊"));
        REQUIRE(WRegex(L"今天{ 3 , 5 }气不错啊").Match(u8"今天天天气不错啊"));
        REQUIRE(Regex16(u8"今天{3, 5\t}气不错啊").Match(u8"今天天天天气不错啊"));
        REQUIRE(Regex32(u8"今天{3,\t5}气不错啊").Match(u8"今天天天天天气不错啊"));

        REQUIRE(!Regex8(u8"今天{3, 5}气不错啊").Match(u8"今天天气不错啊"));
        REQUIRE(!Regex8(u8"今天{3, 5}气不错啊").Match(u8"今天天天天天天气不错啊"));

        REQUIRE_THROWS(Regex8(u8"今天{2, 1}天气不错啊").Match(u8"今天气不错啊"));
        REQUIRE_THROWS(Regex8(u8"今天{0, 0}天气不错啊").Match(u8"今天气不错啊"));
        REQUIRE_THROWS(Regex8(u8"今天{0}天气不错啊").Match(u8"今气不错啊"));
        REQUIRE_NOTHROW(Regex8(u8"今天{0, 1}天气不错啊").Match(u8"今天气不错啊"));

        {
            auto m = Regex8(u8"&abc&(def)+&xyz&").Match(u8"abcdefdefxyz");
            REQUIRE((m && m(0, 1) == u8"abc" && m(1, 2) == u8"defdef" && m(2, 3) == u8"xyz"));
        }

        REQUIRE(Regex8(u8"b").Search(u8"abc"));
        REQUIRE(Regex16(u8"b+").Search(u8"abbbc"));

        REQUIRE(Regex8("mine|craft").Match("mine"));
        REQUIRE(Regex8("mine|craft").Match("craft"));
        REQUIRE(!Regex8("mine|craft").Match("minecraft"));

        REQUIRE(Regex8("[a-z]+").Match("minecraft"));
        REQUIRE(Regex8("\\d+").Match("123456"));
        REQUIRE(!Regex8("\\d+").Match("12a3456"));
        REQUIRE(Regex8("\\w+").Match("variableName"));
        REQUIRE(Regex8("\\w+").Match("variable_name"));
        REQUIRE(Regex8("\\w+").Match("0_variable_name_1"));
        REQUIRE(!Regex8("\\w+").Match("0_va riable_name_1"));
        REQUIRE(Regex8("\\s+").Match("\n  \t \r "));
        REQUIRE(!Regex8("\\s+").Match("\n  !\t \r "));
        REQUIRE(!Regex8("[a-z]+").Match("Minecraft"));
        REQUIRE(!Regex8("@{![a-z]}+").Match("abcDefg"));

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
            REQUIRE((n && n(0, 1) == "ddeeff"));
        }

        {
            auto m = Regex8("&.*&\\.&@{!\\.}*&").Match("abc.x.y.z");
            REQUIRE((m && m(0, 1) == "abc.x.y" && m(2, 3) == "z"));
        }

        REQUIRE(Regex8(u8"mine").Search(u8"abcminecraft"));
        REQUIRE(Regex32(u8"^mine").Search(u8"abcminecraft") == false);

        REQUIRE(Regex8("@{[a-p]&[h-t]&!k}+").Match("hijlmnop"));
        REQUIRE(!Regex8("@{[a-p]&[h-t]&!k}+").Match("hijklmnop"));
    }
}
