#pragma once

#include <chrono>

#include "../Misc/Common.h"

namespace AGZ {

/**
 * @brief 仅用于计时的时钟类
 */
class Timer
{
    using Clock = std::chrono::high_resolution_clock;

    std::chrono::time_point<Clock> start_;

public:

    //! 被创建时会自动开始计时
    Timer() { Restart(); }

    //! 重新开始计时
    void Restart() { start_ = Clock::now(); }

    //! 从上一次开始计时起过了多少秒
    uint64_t Seconds() const
    {
        return std::chrono::duration_cast<
                    std::chrono::seconds>
                        (Clock::now() - start_).count();
    }

    //! 从上一次开始计时起过了多少毫秒
    uint64_t Milliseconds() const
    {
        return std::chrono::duration_cast<
                    std::chrono::milliseconds>
                        (Clock::now() - start_).count();
    }

    //! 从上一次开始计时起过了多少微秒
    uint64_t Microseconds() const
    {
        return std::chrono::duration_cast<
                    std::chrono::microseconds>
                        (Clock::now() - start_).count();
    }
};

} // namespace AGZ
