#include <Utils/FileSys.h>

#include "Catch.hpp"

using namespace AGZ::FileSys;

TEST_CASE("FileSys")
{
    SECTION("Path")
    {
        REQUIRE(Path8("C:\\Minecraft/XYZ", Path8::Windows).IsAbsolute());
        REQUIRE(!Path8("Minecraft/XYZ", Path8::Windows).IsAbsolute());
        REQUIRE(Path8("/Minecraft/XYZ", Path8::Linux).IsAbsolute());

        REQUIRE(WPath(L"A/B/C").HasFilename());
        REQUIRE(!WPath(L"A/B/C/").HasFilename());
        REQUIRE(!WPath(L"A/B/C/D\\", WPath::Windows).HasFilename());
        REQUIRE(!WPath(L"A/B/C", false).HasFilename());

        REQUIRE(WPath(L"A/B/C").GetFilename() == L"C");
        REQUIRE(WPath(L"A/B/C\\D", WPath::Windows).GetFilename() == L"D");

        REQUIRE(WPath(L"abc.txt").GetExtension() == L"txt");
        REQUIRE(WPath(L"abc.txt.").GetExtension().Empty());

        {
            WPath p(L"A/B/C/a.b.txt");
            p.SetExtension(L"rar");
            REQUIRE(p.ToStr(WPath::Linux) == L"A/B/C/a.b.rar");
        }

        {
            WPath p(L"A/B/C/a.b.txt.");
            p.SetExtension(L"rar");
            REQUIRE(p.ToStr(WPath::Linux) == L"A/B/C/a.b.txt..rar");
        }

        {
            WPath p(L"今天天气不错\\你好啊\\", WPath::Windows);
            p.ToAbsolute();
            REQUIRE(p.IsAbsolute());
        }
    }
}
