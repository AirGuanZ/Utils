#pragma once

#include <cstdint>
#include <random>

#include "../Common.h"

AGZ_NS_BEG(AGZ::Math::Random)

using SharedEngine = std::default_random_engine;

struct SharedRandomEngine_t
{
    AGZ_FORCE_INLINE auto &GetEng() { return eng; }
    SharedEngine eng;
};

template<typename T, typename S> struct IntUniform_t
{
    static AGZ_FORCE_INLINE T Eval(T minv, T maxv, S &rng)
    {
        return std::uniform_int_distribution<T>(minv, maxv)(rng.GetEng());
    }
};

template<typename T, typename S> struct RealUniform_t
{
    static AGZ_FORCE_INLINE T Eval(T minv, T maxv, S &rng)
    {
        return std::uniform_real_distribution<T>(minv, maxv)(rng.GetEng());
    }
};

#define MAKE_INT_UNIFORM_T(T) \
    template<typename S> struct Uniform_t<T, S> : IntUniform_t<T, S> { }
#define MAKE_REAL_UNIFORM_T(T) \
    template<typename S> struct Uniform_t<T, S> : RealUniform_t<T, S> { }

template<typename T, typename S> struct Uniform_t;

MAKE_INT_UNIFORM_T(std::int16_t);
MAKE_INT_UNIFORM_T(std::int32_t);
MAKE_INT_UNIFORM_T(std::int64_t);

MAKE_INT_UNIFORM_T(std::uint16_t);
MAKE_INT_UNIFORM_T(std::uint32_t);
MAKE_INT_UNIFORM_T(std::uint64_t);

MAKE_REAL_UNIFORM_T(float);
MAKE_REAL_UNIFORM_T(double);

#undef MAKE_INT_UNIFORM_T
#undef MAKE_REAL_UNIFORM_T

inline thread_local SharedRandomEngine_t SHARED_RNG;

template<typename T, typename S = SharedRandomEngine_t>
AGZ_FORCE_INLINE T Uniform(T min, T max, S &rng = SHARED_RNG)
{
    return Uniform_t<T, S>::Eval(min, max, rng);
}

AGZ_FORCE_INLINE void SetSharedSeed(SharedEngine::result_type seed)
{
    SHARED_RNG.GetEng().seed(seed);
}

AGZ_NS_END(AGZ::Math::Random)
