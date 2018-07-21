#include <algorithm>

#include <Buffer/Prelude.h>
#include <Time/Prelude.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace Buffer;
using namespace std;

TEST_CASE("Buffer")
{
    SECTION("Buffer2D")
    {
        Buffer2D<int> buf0 = Buffer2D<int>::New(640, 480);

        Buffer2D<int> buf1 = Buffer2D<int>::FromFn(640, 480,
            [](size_t x, size_t y, int *buf)
        {
            *buf = static_cast<int>(x * y);
        });
    }
}
