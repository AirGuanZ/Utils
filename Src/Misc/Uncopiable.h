#pragma once

#include "Common.h"

AGZ_NS_BEG(AGZ)

class Uncopiable
{
public:

    Uncopiable()                             = default;
    Uncopiable(const Uncopiable&)            = delete;
    Uncopiable &operator=(const Uncopiable&) = delete;
};

AGZ_NS_END(AGZ)
