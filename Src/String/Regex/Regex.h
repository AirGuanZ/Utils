#pragma once

#include <vector>

#include "../../Misc/Common.h"
#include "../../Range/Iterator.h"
#include "../String.h"
#include "PikeVM.h"
#include "VMEngine.h"

// Regular expression

/*
    concept RegexEngine<CS>
    {
        static constexpr bool SupportSubmatching;

        RegexEngine(const StringView<CS> &regex);

        if constexpr(!SupportSubmatching)
        {
            bool Match(const StringView<CS> &dst);
        }
        else
        {
            std::optional<std::vector<size_t>>
                Match(const StringView<CS> &dst);
            std::optional<std::pair<std::pair<size_t, size_t>,
                                    std::vector<size_t>>>
                Search(const StringView<CS> &dst);
        }
    }
*/

AGZ_NS_BEG(AGZ)

// TODO

AGZ_NS_END(AGZ)
