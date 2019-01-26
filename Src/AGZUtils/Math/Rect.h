#pragma once

#include "../Misc/Common.h"
#include "Scalar.h"
#include "Vec2.h"

namespace AGZ::Math {

template<typename T>
class Rect
{
public:

    Vec2<T> low, high;

    auto GetDeltaX() const noexcept
    {
        return high.x - low.x;
    }

    auto GetDeltaY() const noexcept
    {
        return high.y - low.y;
    }

    auto GetArea() const noexcept
    {
        auto delta = high - low;
        return Max<T>(delta.x, 0) * Max<T>(delta.y, 0);
    }

    auto GetCentre() const noexcept
    {
        return (low + high) / 2;
    }

    Rect<T> Intersect(const Rect<T> &other) const noexcept
    {
        return {
            { Min(low.x,  other.low.x),  Min(low.y,  other.low.y),  Min(low.z,  other.low.z)  },
            { Max(high.x, other.high.x), Max(high.y, other.high.y), Max(high.z, other.high.z) }
        };
    }

    std::optional<Rect<T>> Union(const Rect<T> &other) const noexcept
    {
        Rect<T> ret = {
            { Max(low.x,  other.low.x),  Max(low.y,  other.low.y),  Max(low.z,  other.low.z)  },
            { Min(high.x, other.high.x), Min(high.y, other.high.y), Min(high.z, other.high.z) }
        };
        if(ret.low.x > ret.high.x || ret.low.y > ret.high.y || ret.low.z > ret.high.z)
            return std::nullopt;
        return ret;
    }
};

}
