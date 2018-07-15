#include <iostream>

#include <Math/Prelude.h>
#include <SIMD/Prelude.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace Math;
using namespace std;

TEST_CASE("SIMD")
{
    SECTION("Basics")
    {
        __m128 a4 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
        __m128 b4 = _mm_set_ps(2.0f, 3.0f, 4.0f, 5.0f);
        __m128 s4 = _mm_add_ps(a4, b4);
        REQUIRE(ApproxEq(Vec4f(s4.m128_f32), Vec4f(9.0f, 7.0f, 5.0f, 3.0f), 1e-7f));

        alignas(16) float data[4] = { 4.0f, 3.0f, 2.0f, 1.0f };
        __m128 d4 = _mm_load_ps(data);
        d4 = _mm_sqrt_ps(d4);
        _mm_store_ps(data, d4);
        REQUIRE(ApproxEq(Vec4f(data),
                         Vec4f(4.0f, 3.0f, 2.0f, 1.0f).Map<float>(
                            [](float x) { return Sqrt(x); }),
                         1e-5f));
    }
}
