#include <algorithm>

#include <Utils.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace Buf;
using namespace Math;
using namespace std;

TEST_CASE("Buffer")
{
    SECTION("Buffer")
    {
        auto buf = Buffer<int>::FromFn(10, [](size_t i, int *v)
            { *v = static_cast<int>(i); });

        REQUIRE(buf(0) == 0);
        REQUIRE(buf(9) == 9);

        int sum = buf
            .Map<int>([](int *src, int *dst) { *dst = 2 * *src; })
            .Foldl(0, [](int a, int v) { return a + v; });
        REQUIRE(sum == (1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9) * 2);
    }

    SECTION("Buffer2D")
    {
        auto buf0 = Buffer2D<int>::New(640, 480);

        auto buf1 = Buffer2D<int>::FromFn(640, 480,
            [](size_t x, size_t y, int *buf)
        {
            *buf = static_cast<int>(x * y);
        });

        auto buf2 = Buffer2D<float>::FromConstOther(buf1,
            [](const int *src, float *dst)
        {
            *dst = static_cast<float>(*src);
        });

        auto buf3 = buf2.Map<int>(
            [](const float *src, int *dst)
        {
            *dst = static_cast<int>(*src);
        });

        buf0 = std::move(buf1);

        REQUIRE(buf0(45, 46) == 45 * 46);
        REQUIRE(buf0(212, 13) == 212 * 13);
        REQUIRE(ApproxEq(buf2(212, 13), 212.0f * 13.0f, 1e-5f));
    }
}
