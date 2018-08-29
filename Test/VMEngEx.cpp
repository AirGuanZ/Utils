#include <Utils/String.h>

#include "Catch.hpp"

using namespace AGZ;

auto P8(const WStr &str)
{
    return VMEngExImpl::Parser<WUTF>().Parse(str);
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
}
