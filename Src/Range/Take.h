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
        typename R::Iterator::difference_type num_;

    public:

        using Iterator = typename R::Iterator;

        TakeImpl(R range, typename Iterator::difference_type num)
            : range_(std::move(range)), num_(num)
        {
            AGZ_ASSERT(num >= 0);
        }

        Iterator begin() const
        {
            return std::begin(range_);
        }

        Iterator end() const
        {
            if constexpr(std::is_base_of_v<
                std::random_access_iterator_tag,
                typename Iterator::iterator_category>)
            {
                return std::begin(range_)
                     + std::min(std::end(range_) - std::begin(range_),
                                num_);
            }
            else
            {
                decltype(num_) n = 0;
                Iterator ret = std::begin(range_);
                while(n < num_ && ret != std::end(range_))
                    ++ret, ++n;
                return ret;
            }
        }
    };

    struct TakeRHS { size_t n; };
}

inline RangeAux::TakeRHS Take(size_t n) { return RangeAux::TakeRHS { n }; }

template<typename R>
auto operator|(R &&range, RangeAux::TakeRHS rhs)
{
    using RT = RangeAux::TakeImpl<std::remove_cv_t<
                            std::remove_reference_t<R>>>;
    return RT(std::forward<R>(range),
        static_cast<typename RT::Iterator::difference_type>(rhs.n));
}

AGZ_NS_END(AGZ)
