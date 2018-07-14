#pragma once

#include <cstdint>
#include <random>

#include "../Common.h"

AGZ_NS_BEG(AGZ)
AGZ_NS_BEG(Math)
AGZ_NS_BEG(Random)

using SharedEngine = std::minstd_rand;

struct SharedRandomEngine_t
{
    static AGZ_FORCE_INLINE auto &GetEng()
    {
        static thread_local SharedEngine eng;
        return eng;
    }
};

template<typename T, typename S> struct IntUniform_t
{
    static AGZ_FORCE_INLINE T Eval(T minv, T maxv)
    {
        return std::uniform_int_distribution<T>(minv, maxv)(S::GetEng());
    }
};

template<typename T, typename S> struct RealUniform_t
{
    static AGZ_FORCE_INLINE T Eval(T minv, T maxv)
    {
        return std::uniform_real_distribution<T>(minv, maxv)(S::GetEng());
    }
};

#define MAKE_INT_UNIFORM_T(T) \
    template<typename S> struct Uniform_t<T, S> : IntUniform_t<T, S> { }
#define MAKE_REAL_UNIFORM_T(T) \
    template<typename S> struct Uniform_t<T, S> : RealUniform_t<T, S> { }

template<typename T, typename S> struct Uniform_t;

MAKE_INT_UNIFORM_T(std::int8_t);
MAKE_INT_UNIFORM_T(std::int16_t);
MAKE_INT_UNIFORM_T(std::int32_t);
MAKE_INT_UNIFORM_T(std::int64_t);

MAKE_INT_UNIFORM_T(std::uint8_t);
MAKE_INT_UNIFORM_T(std::uint16_t);
MAKE_INT_UNIFORM_T(std::uint32_t);
MAKE_INT_UNIFORM_T(std::uint64_t);

MAKE_REAL_UNIFORM_T(float);
MAKE_REAL_UNIFORM_T(double);

#undef MAKE_INT_UNIFORM_T
#undef MAKE_REAL_UNIFORM_T

template<typename T, typename S = SharedRandomEngine_t>
AGZ_FORCE_INLINE T Uniform(T min, T max)
{
    return Uniform_t<T, S>::Eval(min, max);
}

AGZ_FORCE_INLINE void SetSharedSeed(SharedEngine::result_type seed)
{
    SharedRandomEngine_t().GetEng().seed(seed);
}

AGZ_NS_END(Random)
AGZ_NS_END(Math)
AGZ_NS_END(AGZ)
