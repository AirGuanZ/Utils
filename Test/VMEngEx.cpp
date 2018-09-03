#include <Utils/String.h>

#include "Catch.hpp"

using namespace AGZ;

void P8(const WStr &str)
{
    (void)VMEngExImpl::Parser<WUTF>().Parse(str);
}

void Gen(const WStr &str)
{
    uint32_t slotCount;
    (void)VMEngExImpl::Backend<WUTF>().Generate(
        VMEngExImpl::Parser<WUTF>().Parse(str), &slotCount);
}

TEST_CASE("VMEngEx")
{
    SECTION("Parser")
    {
        using VMEngExImpl::Parser;
        using P = Parser<UTF8<>>;

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
}
