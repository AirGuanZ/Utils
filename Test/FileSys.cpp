//#include <AGZUtils/Utils/FileSys.h>
//
//#include "Catch.hpp"
//
//using namespace AGZ;
//using namespace FileSys;
//
//TEST_CASE("BinaryView")
//{
//    SECTION("Memory")
//    {
//
//    }
//}
//
//TEST_CASE("FileSys")
//{
//    SECTION("File")
//    {
//        REQUIRE(!File::GetWorkingDirectory().empty());
//        REQUIRE(Path8(File::GetWorkingDirectory()).IsAbsolute());
//        REQUIRE(!Path8(File::GetWorkingDirectory(), false).HasFilename());
//    }
//
//    SECTION("Path")
//    {
//        REQUIRE(Path8("C:\\Minecraft/XYZ", Path8::Windows).IsAbsolute());
//        REQUIRE(!Path8("Minecraft/XYZ", Path8::Windows).IsAbsolute());
//        REQUIRE(Path8("/Minecraft/XYZ", Path8::Linux).IsAbsolute());
//
//        REQUIRE(WPath(L"A/B/C").HasFilename());
//        REQUIRE(!WPath(L"A/B/C/").HasFilename());
//        REQUIRE(!WPath(L"A/B/C/D\\", WPath::Windows).HasFilename());
//        REQUIRE(!WPath(L"A/B/C", false).HasFilename());
//
//        REQUIRE(!WPath(L"A/B/C").IsDirectory());
//        REQUIRE(WPath(L"A/B/C/").IsDirectory());
//        REQUIRE(WPath(L"A/B/C/D\\", WPath::Windows).IsDirectory());
//        REQUIRE(WPath(L"A/B/C", false).IsDirectory());
//
//        REQUIRE(WPath(L"A/B/C").HasParent());
//        REQUIRE(!WPath(L"A.txt").HasParent());
//
//        REQUIRE(WPath(L"A/B/C/").IsPrefixOf(WPath(L"A/B/C/D/")));
//        REQUIRE(!WPath(L"A/B/C/D/E/").IsPrefixOf(WPath(L"A/B/C/D/")));
//
//        REQUIRE(WPath(L"A/B/C").GetFilename() == L"C");
//        REQUIRE(WPath(L"A/B/C\\D", WPath::Windows).GetFilename() == L"D");
//
//        REQUIRE(WPath(L"A/B/C").SetFilename(L"a.txt")
//                               .SetExtension(L"rar")
//                               .ToStr(WPath::Linux)
//             == L"A/B/a.rar");
//
//        REQUIRE(WPath(L"abc.txt").GetExtension() == L"txt");
//        REQUIRE(WPath(L"abc.txt.").GetExtension().Empty());
//        REQUIRE(WPath(L"A/B/C/a.b.txt.")
//                    .SetExtension(L"rar")
//                    .ToStr(WPath::Linux) == L"A/B/C/a.b.txt..rar");
//        
//        REQUIRE(WPath(L"A/B/C/").Append(WPath(L"D/E/F"))
//             == WPath(L"A/B/C/D/E/F"));
//
//        REQUIRE(WPath(L"今天天气不错\\你好啊\\", WPath::Windows)
//                    .ToAbsolute()
//                    .IsAbsolute());
//
//        REQUIRE(WPath(L"今天天气不错\\你好啊\\", WPath::Windows)
//                    .ToAbsolute()
//                    .ToRelative()
//                    .IsAbsolute());
//        
//        REQUIRE(WPath(L"A/B/C/D").ToParent() == WPath(L"A/B/C/"));
//        REQUIRE(WPath(L"A/B/C/D/").ToParent() == WPath(L"A/B/C/"));
//
//        REQUIRE(Path8("A/B/C/D").GetPrefix(2) == Path8("A/B/"));
//        REQUIRE(Path8("A/B/C/D").GetPrefix(4) == Path8("A/B/C/D"));
//        REQUIRE(Path8("A/B/C/D").GetPrefix(3).IsRelative());
//    }
//}
