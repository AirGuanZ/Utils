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
        REQUIRE_NOTHROW(P8("[abc]|[a-zA-Zdef0-9][m-x]"));
        REQUIRE_NOTHROW(P8("[abcA-Z]+*??*+"));
    }
}
