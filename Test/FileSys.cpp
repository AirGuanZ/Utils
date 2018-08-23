#pragma once

#include <Utils/FileSys.h>

#include "Catch.hpp"

using namespace AGZ;

TEST_CASE("FileSys")
{
    SECTION("Path")
    {
        REQUIRE(Path8("C:\\Minecraft/XYZ", Path8::Windows).IsAbsolute());
        REQUIRE(!Path8("Minecraft/XYZ", Path8::Windows).IsAbsolute());
        REQUIRE(Path8("/Minecraft/XYZ", Path8::Linux).IsAbsolute());

        REQUIRE(WPath(L"A/B/C").HasFilename());
        REQUIRE(!WPath(L"A/B/C/").HasFilename());
        REQUIRE(!WPath(L"A/B/C", false).HasFilename());
    }
}
