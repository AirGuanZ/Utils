#include <Utils/FileSys.h>
#include <Utils/Platform.h>

#include "Catch.hpp"

using namespace AGZ;

TEST_CASE("Platform")
{
    SECTION("WorkingDirectory")
    {
        REQUIRE(!Platform::GetWorkingDirectory().Empty());
        REQUIRE(WPath(Platform::GetWorkingDirectory()).IsAbsolute());
        REQUIRE(!WPath(Platform::GetWorkingDirectory()).HasFilename());
    }
}
