#pragma once

#include "../../Misc/Common.h"

namespace AGZ::UTF
{

template<typename TCharIn, typename TCharOut,
    std::enable_if_t<sizeof(TCharIn) == 1 && sizeof(TCharOut) == 4, int> = 0>
std::pair<const TCharIn*, TCharOut> NextUnicodeFromUTF8(const TCharIn *in);

} // namespace AGZ::UTF
