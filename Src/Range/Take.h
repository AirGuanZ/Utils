#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

namespace RangeAux
{
    template<typename R>
    class TakeImpl
    {
        R range_;
        typename R::Iterator end_;

    public:

        using Iterator = typename R::Iterator;

        TakeImpl(R range, typename Iterator::difference_type num)
            : range_(std::move(range))
        {
            end_ = AdvanceTo(std::begin(range_), std::end(range_), num);
        }

        Iterator begin() const
        {
            return std::begin(range_);
        }

        Iterator end() const
        {
            return end_;
        }
    };

    struct TakeRHS { size_t n; };
}

inline RangeAux::TakeRHS Take(size_t n) { return RangeAux::TakeRHS { n }; }

template<typename R>
auto operator|(R &&range, RangeAux::TakeRHS rhs)
{
    using RT = RangeAux::TakeImpl<remove_rcv_t<R>>;
    return RT(std::forward<R>(range),
        static_cast<typename RT::Iterator::difference_type>(rhs.n));
}

AGZ_NS_END(AGZ)
