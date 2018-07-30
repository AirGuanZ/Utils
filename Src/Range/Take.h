#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

template<typename R>
class TakeImpl
{
    R range_;
    typename R::Iterator::difference_type num_;

public:

    using It = typename R::Iterator;

    Take(R range, typename It::difference_type num)
        : range_(std::move(range)), num_(num)
    {
        AGZ_ASSERT(num >= 0);
    }

    It begin() const
    {
        return std::begin(range_);
    }

    It end() const
    {
        if constexpr(std::is_base_of_v<
            std::random_access_iterator_tag,
            typename It::iterator_category>)
        {
            return std::begin(range_)
                 + std::min(std::end(range_) - std::begin(range_),
                            num_);
        }
        else
        {
            decltype(num_) n = 0;
            It ret = std::begin(range_);
            while(n < num_ && ret != std::end(range_))
                ++ret, ++n;
            return ret;
        }
    }
};

AGZ_NS_END(AGZ)
