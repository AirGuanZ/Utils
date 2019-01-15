#include <algorithm>
#include <string>

#include <AGZUtils/Utils/Math.h>
#include <AGZUtils/Utils/Misc.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace Math;
using namespace std;

TEST_CASE("Buffer")
{
    SECTION("COWObject")
    {
        COWObject<string> s0("Minecraft");
        COWObject<string> s1 = s0;
        REQUIRE(s0.Refs() == 2);
        REQUIRE(*s1 == "Minecraft");
        s1.Mutable() = "Dark Souls";
        REQUIRE((s0.Refs() == 1 && s1.Refs() == 1));
        s0.Release();
        COWObject<string> s2 = std::move(s1);
        REQUIRE((s0.Refs() == 0 && s1.Refs() == 0));

        REQUIRE(s2->length() == 10);
    }
}
