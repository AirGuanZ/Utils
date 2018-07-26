#pragma once

#include <cstdint>
#include <random>

#include "../Common.h"

AGZ_NS_BEG(AGZ::Math::Random)

using SharedEngine = std::default_random_engine;

struct SharedRandomEngine_t
{
    auto &GetEng() { return eng; }
    SharedEngine eng;
};

template<typename T, typename S> struct IntUniform_t
{
    static T Eval(T minv, T maxv, S &rng)
    {
        return std::uniform_int_distribution<T>(minv, maxv)(rng.GetEng());
    }
};

template<typename T, typename S> struct RealUniform_t
{
    static T Eval(T minv, T maxv, S &rng)
    {
        return std::uniform_real_distribution<T>(minv, maxv)(rng.GetEng());
    }
};

template<typename T, typename S> struct RealNormal_t
{
    static T Eval(T mean, T stddev, S &rng)
    {
        return std::normal_distribution<T>(mean, stddev)(rng.GetEng());
    }
};

#define MAKE_INT_UNIFORM_T(T) \
    template<typename S> struct Uniform_t<T, S> : IntUniform_t<T, S> { }
#define MAKE_REAL_UNIFORM_T(T) \
    template<typename S> struct Uniform_t<T, S> : RealUniform_t<T, S> { }
#define MAKE_REAL_NORMAL_T(T) \
    template<typename S> struct Normal_t<T, S> : RealNormal_t<T, S> { }

template<typename T, typename S> struct Uniform_t;
template<typename T, typename S> struct Normal_t;

MAKE_INT_UNIFORM_T(std::int16_t);
MAKE_INT_UNIFORM_T(std::int32_t);
MAKE_INT_UNIFORM_T(std::int64_t);

MAKE_INT_UNIFORM_T(std::uint16_t);
MAKE_INT_UNIFORM_T(std::uint32_t);
MAKE_INT_UNIFORM_T(std::uint64_t);

MAKE_REAL_UNIFORM_T(float);
MAKE_REAL_UNIFORM_T(double);

MAKE_REAL_NORMAL_T(float);
MAKE_REAL_NORMAL_T(double);

#undef MAKE_INT_UNIFORM_T
#undef MAKE_REAL_UNIFORM_T
#undef MAKE_REAL_NORMAL_T

inline thread_local SharedRandomEngine_t SHARED_RNG;

template<typename T, typename S = SharedRandomEngine_t>
T Uniform(T min, T max, S &rng = SHARED_RNG)
{
    return Uniform_t<T, S>::Eval(min, max, rng);
}

template<typename T, typename S = SharedRandomEngine_t>
T Normal(T mean, T stddev, S &rng = SHARED_RNG)
{
    return Normal_t<T, S>::Eval(mean, stddev, rng);
}

AGZ_INLINE void SetSharedSeed(SharedEngine::result_type seed)
{
    SHARED_RNG.GetEng().seed(seed);
}

AGZ_NS_END(AGZ::Math::Random)
