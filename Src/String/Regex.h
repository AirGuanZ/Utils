#pragma once

#include <vector>

#include "../Misc/Common.h"
#include "../Range/Iterator.h"
#include "String.h"

// Regular expression VM
// See https://swtch.com/~rsc/regexp/regexp2.html

AGZ_NS_BEG(AGZ)

// TODO

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
    Match(Str &&whole, std::vector<StrView> &&segs);
    Match(const Self &copyFrom);
    Match(Self &&moveFrom);

    Self &operator=(const Self &copyFrom);
    Self &operator=(Self &&moveFrom);

    size_t Size() const;

    const Str &GetWholeMatch() const;

    const StrView &GetMatchSegments(size_t idx) const;
    const StrView &operator[](size_t idx) const;

    Iterator begin() const;
    Iterator end()   const;
};

AGZ_NS_END(AGZ)
