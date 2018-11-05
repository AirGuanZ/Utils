#pragma once

#include "Common.h"

namespace AGZ {

class Uncopiable
{
public:

    Uncopiable()                             = default;
    Uncopiable(const Uncopiable&)            = delete;
    Uncopiable &operator=(const Uncopiable&) = delete;
};

} // namespace AGZ
