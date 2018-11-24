#include <Utils/Config.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace std;

static const Str8 S0 = u8R"___(
Window =
{ # This is a comment
    Title = "AGZ Application";
    Size = { Width = 640; Height = 480; };
    Pos = { Left = 0; Top = 0; };
};
# This is a comment
Array = (( "Minecraft", 123, False), 996.1234, ());
## Hello?!

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

Window = 
{
    Bordered = True;
    Visible = False;
};

Angle = Deg(70.0);
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

            REQUIRE(root.Find("Window.Title")->AsValue() == "AGZ Application");
            REQUIRE(root.Find("Others.Integer")->AsValue().Parse<int>() == 27);
            REQUIRE(root["Window.Bordered"].AsValue() == "True");
            REQUIRE(root["Window.Visible"].AsValue() == "False");
            REQUIRE(root["Angle"].AsArray().GetTag() == "Deg");
            REQUIRE(root["Angle"].AsArray().At(0)->AsValue() == "70.0");
        }
    }
}
