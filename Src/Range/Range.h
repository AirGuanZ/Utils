#pragma once

#include <iterator>
#include <type_traits>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

template<typename R,
         std::enable_if_t<
            std::is_base_of_v<
                std::random_access_iterator_tag,
                typename R::Iterator::iterator_category>,
            int> = 0>
typename R::Iterator::difference_type RangeSize(const R &range)
{
    return std::end(range) - std::begin(range);
}

AGZ_NS_END(AGZ)
