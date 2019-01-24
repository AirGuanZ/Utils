#include <AGZUtils/String/Std/StdStr.h>

#include "Catch.hpp"

using namespace AGZ::Str;

TEST_CASE("StdStr")
{
    SECTION("Char")
    {
        REQUIRE(IsWhitespace(' '));
        REQUIRE(IsWhitespace('\n'));
        REQUIRE(!IsWhitespace('a'));
        REQUIRE(!IsWhitespace('?'));

        REQUIRE(IsLower('a'));
        REQUIRE(IsLower('z'));
        REQUIRE(!IsLower('A'));
        REQUIRE(!IsLower('?'));

        REQUIRE(IsUpper('A'));
        REQUIRE(IsUpper('Z'));
        REQUIRE(!IsUpper('a'));
        REQUIRE(!IsUpper('?'));

        REQUIRE(IsAlpha('A'));
        REQUIRE(IsAlpha('Z'));
        REQUIRE(IsAlpha('a'));
        REQUIRE(!IsAlpha('?'));

        REQUIRE(ToUpper(u8"今天天气不错; Dark Souls !") == u8"今天天气不错; DARK SOULS !");
        REQUIRE(ToLower("Dark Souls ?") == "dark souls ?");
    }

    SECTION("Trim")
    {
        REQUIRE(TrimLeft(u8"  你好啊") == u8"你好啊");
        REQUIRE(TrimLeft(u8"") == u8"");
        REQUIRE(TrimRight(u8"你好啊  ") == u8"你好啊");
        REQUIRE(Trim(u8"  今天天气不错\t\t ") == u8"今天天气不错");
        REQUIRE(Trim(u8"12 8 今天天气不错\t456\t ", [](char c) { return IsWhitespace(c) || IsDemDigit(c); }) == u8"今天天气不错");
    }

    SECTION("Replace")
    {
        REQUIRE(Replace("Minecraft", "e", "haha") == "Minhahacraft");
        REQUIRE(Replace("aaa", "a", "a") == "aaa");
    }

    SECTION("Join")
    {
        std::vector<std::string> strs = { "1", "2", "3", "4" };
        REQUIRE(Join('+', strs.begin(), strs.end()) == "1+2+3+4");
        REQUIRE(Join(" + ", strs.begin(), strs.end()) == "1 + 2 + 3 + 4");
        strs.clear();
        REQUIRE(Join(" + ", strs.begin(), strs.end()).empty());
    }

    SECTION("Split")
    {
        {
            std::vector<std::string_view> strs;
            size_t size = Split("mine craft is  a good game", std::back_inserter(strs));
            REQUIRE(size == 6);
            REQUIRE(strs == std::vector<std::string_view>{ "mine", "craft", "is", "a", "good", "game" });

            strs.clear();
            size = Split("mine craft is  a good game", std::back_inserter(strs), false);
            REQUIRE(size == 7);
            REQUIRE(strs == std::vector<std::string_view>{ "mine", "craft", "is", "", "a", "good", "game" });
        }

        {
            std::vector<std::string_view> strs;
            size_t size = Split("ab4cd5ef6gh", [](char c) { return IsDemDigit(c); }, std::back_inserter(strs));
            REQUIRE(size == 4);
            REQUIRE(strs == std::vector<std::string_view>{ "ab", "cd", "ef", "gh" });
        }
    }

    SECTION("From & To")
    {
        REQUIRE(To<char>(8) == "8");
        REQUIRE(From<double>(To<char>(36.2)) == 36.2);
        REQUIRE(To<char>(-1) == "-1");
        REQUIRE(To<char>("minecraft") == "minecraft");
    }

    SECTION("UTF")
    {
        REQUIRE(ConvertBetweenUTF<char16_t, char>(ConvertBetweenUTF<char, char16_t>(u8"今天天气不错")) == u8"今天天气不错");
#ifdef AGZ_OS_WIN32
        REQUIRE(ConvertBetweenUTF<char, wchar_t>(u8"今天天气不错") == L"今天天气不错");
#endif
    }

    SECTION("Formatter")
    {
        REQUIRE(TFormatter<char>("abc{0} + {0} = {2}, {1}").Arg(1, 3, 2) == "abc1 + 1 = 2, 3");
        REQUIRE(TFormatter<char>(u8"今天{1}天气{}不错").Arg(0, 1, 2) == u8"今天1天气2不错");
    }

    SECTION("Scanner")
    {
        {
            int a, b;
            REQUIRE(TScanner<char>("abc{}def{}").Scan("abc123def456", a, b));
            REQUIRE((a == 123 && b == 456));
            REQUIRE(TScanner<char>(u8"今天天气不错").Scan(u8"今天天气不错"));
            REQUIRE(!TScanner<char>(u8"今天天气不错").Scan(u8"今天天气很好"));
        }
    }
}
