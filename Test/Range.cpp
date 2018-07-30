#include <iostream>
#include <vector>

#include <Utils.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace std;

TEST_CASE("Range")
{
    SECTION("Seq & Take")
    {
        vector<int> v;
        for(auto i : Seq<int>(1) | Take(5))
            v.push_back(i);
        REQUIRE(v == vector<int>{ 1, 2, 3, 4, 5 });

        v.clear();
        for(auto i : Seq<int>(1, 2) | Take(5))
            v.push_back(i);
        REQUIRE(v == vector<int>{ 1, 3, 5, 7, 9 });

        v.clear();
        for(auto i : Seq<int>(1, -1) | Take(5))
            v.push_back(i);
        REQUIRE(v == vector<int>{ 1, 0, -1, -2, -3 });

        v.clear();
        for(auto i : Seq<int>(1, -1) | Take(5) | Take(100))
            v.push_back(i);
        REQUIRE(v == vector<int>{ 1, 0, -1, -2, -3 });

        v.clear();
        for(auto i : Seq<int>(1, -1) | Take(100) | Take(5))
            v.push_back(i);
        REQUIRE(v == vector<int>{ 1, 0, -1, -2, -3 });
    }
}
