#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <utility>

#include "../Common.h"

AGZ_NS_BEG(AGZ)
AGZ_NS_BEG(Time)

AGZ_NS_BEG(Bench)

template<typename F>
void Bench_impl(F &&func, int repeat)
{
    auto ms = decltype(std::chrono::system_clock::now() - std::chrono::system_clock::now())(0);
    for(int i = 0; i < repeat; ++i)
    {
        auto begin = std::chrono::system_clock::now();
        func();
        ms = ms + std::chrono::system_clock::now() - begin;
    }
    std::cout << (ms / repeat).count() << "ms" << std::endl;
}

class Bench_t
{
public:
    template<typename F>
    AGZ_FORCE_INLINE const Bench_t &Run(F &&func, int repeat = 1) const
    {
        Bench_impl(std::forward<F>(func), repeat);
        return *this;
    }
};

template<typename F>
AGZ_FORCE_INLINE Bench_t Run(F &&func, int repeat = 1)
{
    Bench_impl(std::forward<F>(func), repeat);
    return Bench_t();
}

template<typename F>
AGZ_FORCE_INLINE Bench_t NamedRun(const std::string &name, F &&func, int repeat = 1)
{
    std::cout << "[Benchmark] " << name << " ";
    return Run(std::forward<F>(func), repeat);
}

#define AGZ_TIME_BENCH(N, R, F) (::AGZ::Time::Bench::NamedRun(N, F, R))

AGZ_NS_END(Bench)

AGZ_NS_END(Time)
AGZ_NS_END(AGZ)
