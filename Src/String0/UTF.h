#pragma once

#include "../Misc/Common.h"
#include "UTF8.h"
#include "UTF16.h"
#include "UTF32.h"

AGZ_NS_BEG(AGZ)

// Why does windows use UTF-16...
using WUTF = std::conditional_t<sizeof(wchar_t) == 4, UTF32<>, UTF16<>>;

AGZ_NS_END(AGZ)
