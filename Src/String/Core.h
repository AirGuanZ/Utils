#pragma once

#include "../Misc/Common.h"
#include "../Misc/Option.h"

AGZ_NS_BEG(AGZ::String::Core)

// TODO
template<typename CU, typename CP>
class TStr
{
    const CU *beg_;
    const CU *end_;
    Option<size_t> cpCount_;

public:

    using CharUnit  = CU;
    using CharPoint = CP;
    using Self = TStr<CU, CP>;

    TStr(const CU *beg, const CU *end) : beg_(beg), end_(end) { }
    TStr(const Self &copyFrom) = default;
    ~TStr() = default;

    Self &operator=(const Self &copyFrom) = default;

    size_t Length() const;

    CU CodeUnit(size_t idx) const;
    CP CodePoint(size_t idx) const;

    size_t CodeUnitCount() const;
    size_t CodePointCount() const;
};

AGZ_NS_END(AGZ::String::Core)
