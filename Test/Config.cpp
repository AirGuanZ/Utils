#include <Utils/Config.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace std;

static const Str8 S0 = u8R"___(
Window =
{
    Title = "AGZ Application";
    Size = { Width = 640; Height = 480; };
    Pos = { Left = 0; Top = 0; };
};

Array = (( "Minecraft", 123, False), 996.1234, ());

Students = ({
        Name = "Zhang3";
        Age = 18;
    },
    {
        Name = "Li4";
        Age = 21;
    });

Others =
{
    PI = 3.141592654;
    Strings = ("1", "2", "3");
    Integer = 27;
};
)___";

TEST_CASE("Config")
{
    SECTION("S0")
    {
        Config config;
        REQUIRE(config.LoadFromMemory(S0));

        if(config.IsAvailable())
        {
            auto &root = config.Root();
            REQUIRE(root.Find("Window.Title")->AsValue().GetStr() == "AGZ Application");
            REQUIRE(root.Find("Others.Integer")->AsValue().GetStr().Parse<int>() == 27);
        }
    }
}
