#pragma once

/**
 * @file Math/Random.h
 * @brief 简化std::random中部分组件的使用
 * 
 * 分为三个层次：随机数引擎，随机数生成器，随机数函数
 * 
 * - 随机数引擎：符合std中的random_engine标准的随机数发生引擎
 * - 随机数生成器：将随机数引擎包装成随机数生成器，供随机数函数使用
 * - 随机数函数：用随机数生成器产生满足指定分布的随机数，默认使用的生成器是线程安全的
 * 
 * 一般来说，只需要直接调用随机数函数即可。有特殊需求时，可自行定义随机数生成器作为随机数函数的参数。
 */

#include <cstdint>
#include <ctime>
#include <random>

namespace AGZ::Math::Random {

/** @brief 默认内部随机数引擎 */
using DefaultInternalEngine = std::default_random_engine;

/** @brief 默认随机数生成器类型，使用当前时间来作为随机数引擎的种子 */
template<typename Engine = DefaultInternalEngine>
struct SharedRandomEngine_t
{
    SharedRandomEngine_t()
        : eng(static_cast<typename Engine::result_type>(std::time(nullptr)))
    {
        
    }

    auto &GetEng() { return eng; }
    Engine eng;
};

/**
 * @cond
 */

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

/**
 * @endcond
 */

/**
 * @brief 返回单个线程内共享使用的随机数生成器
 * 
 * 我本来是这么写的：
@code
template<typename Engine = DefaultInternalEngine>
inline thread_local SharedRandomEngine_t<Engine> SHARED_RNG;
@endcode
 * 但是g++有个bug会导致这个SHARED_RNG的构造函数没运行，进而导致RNG内部可能出现非法状态。
 * 这个问题坑掉了我整整一个下午，有必要在这记一笔。
 */
template<typename Engine = DefaultInternalEngine>
SharedRandomEngine_t<Engine> &GetDefaultSharedRNG() noexcept
{
    static thread_local SharedRandomEngine_t<Engine> ret;
    return ret;
}

/**
 * @brief [min, max]上的均匀分布
 * 
 * @note 该函数是线程安全的
 */
template<typename T, typename S = SharedRandomEngine_t<>>
T Uniform(T min, T max, S &rng = GetDefaultSharedRNG<>())
{
    return Uniform_t<T, S>::Eval(min, max, rng);
}

/**
 * @brief 以mean为均值、以stddev为标准差的正态分布
 * 
 * @note 该函数是线程安全的
 */
template<typename T, typename S = SharedRandomEngine_t<>>
T Normal(T mean, T stddev, S &rng = GetDefaultSharedRNG<>())
{
    return Normal_t<T, S>::Eval(mean, stddev, rng);
}

} // namespace AGZ::Math::Random
