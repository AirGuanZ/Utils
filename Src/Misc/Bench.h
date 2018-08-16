#pragma once

#include <chrono>
#include <iostream>
#include <string>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)
AGZ_NS_BEG(Time)

AGZ_NS_BEG(Bench)

template<typename F>
void Bench_impl(int repeat, F &&func)
{
    using namespace std::chrono;
    using C = high_resolution_clock;
    auto ms = decltype(C::now() - C::now())(0);
    for(int i = 0; i < repeat; ++i)
    {
        auto begin = C::now();
        func();
        ms = ms + C::now() - begin;
    }
    std::cout << "[Repeat] " << repeat << " [Average Time] "
              << (duration_cast<milliseconds>(ms) / repeat).count()
              << "ms" << std::endl;
}

class Bench_t
{
public:

    template<typename F>
    const Bench_t &Run(int repeat, F &&func) const
    {
        Bench_impl(repeat, std::forward<F>(func));
        return *this;
    }

    template<typename F>
    const Bench_t &Run(const std::string &name,
                                        int repeat, F &&func) const
    {
        std::cout << "[Benchmark] " << name << " ";
        Bench_impl(repeat, std::forward<F>(func));
        return *this;
    }
};

template<typename F>
Bench_t Run(int repeat, F &&func)
{
    Bench_impl(repeat, std::forward<F>(func));
    return Bench_t();
}

template<typename F>
Bench_t Run(const std::string &name, int repeat, F &&func)
{
    return Bench_t().Run(name, repeat, std::forward<F>(func));
}

AGZ_NS_END(Bench)

AGZ_NS_END(Time)
AGZ_NS_END(AGZ)
