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

        v.clear();
        for(auto i : Between(1, 6))
            v.push_back(i);
        REQUIRE(v == vector<int>{ 1, 2, 3, 4, 5 });

        v.clear();
        auto Square = [](int v) { return v * v; };
        for(auto i : Between(1, 6) | Map(Square))
            v.push_back(i);
        REQUIRE(v == vector<int>{ 1, 4, 9, 16, 25 });

        auto addInt = [](int a, int b) { return a + b; };
        REQUIRE((Between(1, 4) | Reduce(0, addInt)) == 1 + 2 + 3);

        v.clear();
        auto IsEven = [](int v) { return v % 2 == 0; };
        for(auto i : Between(1, 7) | Filter(IsEven))
            v.push_back(i);
        REQUIRE(v == vector<int>{ 2, 4, 6 });

        REQUIRE((Between(0, 100) | Count()) == 100);
        REQUIRE((Between(0, 100) | CountIf(IsEven)) == 50);
    }
}
