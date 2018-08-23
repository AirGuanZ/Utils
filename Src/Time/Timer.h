#pragma once

#include <chrono>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

class Timer
{
    using Clock = std::chrono::high_resolution_clock;

    std::chrono::time_point<Clock> start_;

public:

    Timer() { Restart(); }

    void Restart() { start_ = Clock::now(); }

    uint64_t Seconds() const
    {
        return std::chrono::duration_cast<
                    std::chrono::seconds>
                        (Clock::now() - start_).count();
    }

    uint64_t Milliseconds() const
    {
        return std::chrono::duration_cast<
                    std::chrono::milliseconds>
                        (Clock::now() - start_).count();
    }

    uint64_t Microseconds() const
    {
        return std::chrono::duration_cast<
                    std::chrono::microseconds>
                        (Clock::now() - start_).count();
    }
};

AGZ_NS_END(AGZ)
