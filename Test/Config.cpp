#include <Utils/Config.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace std;

static const Str8 S0 = R"___(
window =
{
    title = "My Application";
    size = { w = 640; h = 480; };
    pos = { x = 350; y = 250; };
};

list = ( ( "abc", 123, true ), 1.234, ( ) );

books = ({
        title = "Treasure Island";
        author = "Robert Louis Stevenson";
        price = 29.95;
        qty = 5;
    },
    {
        title = "Snow Crash";
        author = "Neal Stephenson";
        price = 9.99;
        qty = 8;
    });

misc =
{
    pi = 3.141592654;
    bigint = 9223372036854775807L;
    columns = ("Last Name", "First Name", "MI");
    bitmask = 0x1FC3;
    umask = 27;
};
)___";

TEST_CASE("Config")
{
    SECTION("S0")
    {
        Config config;
        REQUIRE(config.LoadFromMemory(S0));

        auto &root = config.Root();
        REQUIRE(root.Find("window.title")->AsValue().GetStr() == "My Application");
        REQUIRE(root.Find("misc.umask")->AsValue().GetStr().Parse<int>() == 27);
    }
}
