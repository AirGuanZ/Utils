#pragma once

#include <vector>

#include "../../Misc/Common.h"
#include "../../Range/Iterator.h"
#include "../String.h"
#include "PikeVM.h"

// Regular expression

/*
    concept RegexEngine<CS>
    {
        static constexpr bool SupportSubmatching;

        RegexEngine(const StringView<CS> &regex);

        if constexpr(SupportSubmatching)
        {
            bool Match(const StringView<CS> &dst) const;
        }
        else
        {
            std::pair<bool, std::vector<std::pair<size_t, size_t>>>
                Match(const StringView<CS> &dst) const;
        }
    }
*/

AGZ_NS_BEG(AGZ::RegexImpl)

template<typename CS>
class Match
{
    String<CS> whole_;
    std::vector<typename String<CS>::View> segs_;

public:

    using Str      = String<CS>;
    using StrView  = typename Str::View;
    using Self     = Match<CS>;
    using Iterator = GetIteratorType<decltype(segs_)>;

    Match();
    Match(Str &&whole, std::vector<std::pair<size_t, size_t>> &&segs);
    Match(const Self &copyFrom);
    Match(Self &&moveFrom);

    Self &operator=(const Self &copyFrom);
    Self &operator=(Self &&moveFrom);

    size_t Size() const
    {
        return segs_.size();
    }

    const Str &GetWholeMatch() const
    {
        return whole_;
    }

    const StrView &GetMatchSegments(size_t idx) const
    {
        return (*this)[idx];
    }

    const StrView &operator[](size_t idx) const
    {
        AGZ_ASSERT(idx < segs_.size());
        return segs_[idx];
    }

    Iterator begin() const
    {
        return segs_.begin();
    }

    Iterator end()   const
    {
        return segs_.end();
    }
};

AGZ_NS_END(AGZ::RegexImpl)
