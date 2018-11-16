#pragma once

#include <chrono>

#include "../Misc/Common.h"

namespace AGZ {

/**
 * @brief 仅用于计时的时钟类
 */
class Clock
{
    using InternalClock = std::chrono::high_resolution_clock;

    std::chrono::time_point<InternalClock> start_;

public:

    //! 被创建时会自动开始计时
    Clock() { Restart(); }

    //! 重新开始计时
    void Restart() { start_ = InternalClock::now(); }

    //! 从上一次开始计时起过了多少秒
    uint64_t Seconds() const
    {
        return std::chrono::duration_cast<
                    std::chrono::seconds>
                        (InternalClock::now() - start_).count();
    }

    //! 从上一次开始计时起过了多少毫秒
    uint64_t Milliseconds() const
    {
        return std::chrono::duration_cast<
                    std::chrono::milliseconds>
                        (InternalClock::now() - start_).count();
    }

    //! 从上一次开始计时起过了多少微秒
    uint64_t Microseconds() const
    {
        return std::chrono::duration_cast<
                    std::chrono::microseconds>
                        (InternalClock::now() - start_).count();
    }
};

} // namespace AGZ
