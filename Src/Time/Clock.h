#pragma once

#include "../Common.h"
#include "../Result/Option.h"

AGZ_NS_BEG(AGZ::Time)

#ifdef _WIN32

#include <Windows.h>

template<typename R>
class Clock
{
public:

    using Representation = R;
    using Self = Clock<R>;

    Clock() : start_(Self::Now()) { }
    Clock(const Clock&) = default;
    ~Clock() = default;

    AGZ_FORCE_INLINE void Restart() { start_ = Self::Now(); }

    AGZ_FORCE_INLINE R ElaspedTime() { return Self::Now() - start_; }

private:

    static R GetRatio()
    {
        if(ratio_.IsNone())
        {
            LARGE_INTEGER freq;
            QueryPerformanceFrequency(&freq);
            ratio_ = Some(R(R(1000) / freq.QuadPart));
        }
        return ratio_.Unwrap();
    }

    static R Now()
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return R(ratio_ * now.QuadPart);
    }

    static Option<R> ratio_;

    R start_;
};

#else
    #error "Clock unimplemented"
#endif

AGZ_NS_END(AGZ::Time)
