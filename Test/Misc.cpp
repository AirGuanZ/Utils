#pragma once

#include <type_traits>
#include <Utils.h>

#include "Catch.hpp"

using namespace AGZ::TypeOpr;

TEST_CASE("Misc")
{
    SECTION("TypeOpr")
    {
        static_assert(std::is_same_v<Add<int, int>, int>);
        static_assert(std::is_same_v<Add<int, float>, float>);
        static_assert(std::is_same_v<Div<float, double>, double>);
        static_assert(std::is_same_v<Equal<int, float>, bool>);
        static_assert(std::is_same_v<BitNot<int>, int>);
    }
}
