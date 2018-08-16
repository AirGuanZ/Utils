#pragma once

#include <limits>
#include <utility>
#include <vector>

#include "../../Misc/Common.h"
#include "../../Range/Iterator.h"
#include "../String.h"
#include "VMEngine.h"

// Regular expression

/*
    concept RegexEngine<CS>
    {
        std::optional<std::vector<size_t>>
            Match(const StringView<CS> &dst);
        std::optional<std::pair<std::pair<size_t, size_t>,
                                std::vector<size_t>>>
            Search(const StringView<CS> &dst);
    }
*/

AGZ_NS_BEG(AGZ)

template<typename CS>
class Regex;

template<typename CS>
class Match
{
public:

    using Interval = std::pair<size_t, size_t>;
    using Self = Match<CS>;

    friend class Regex<CS>;

    Match()
        : interval_(0, std::numeric_limits<size_t>::max())
    {

    }

    Match(const Self &copyFrom)
        : whole_(copyFrom.whole_),
          interval_(copyFrom.interval_),
          savePoints_(copyFrom.savePoints_)
    {

    }

    Match(Self &&moveFrom)
        : whole_(std::move(moveFrom.whole_)),
          interval_(moveFrom.interval_),
          savePoints_(std::move(moveFrom_.savePoints))
    {
        moveFrom.interval_.second = std::numeric_limits<size_t>::max();
    }

    ~Match() = default;

    Self &operator=(const Self &copyFrom)
    {
        whole_ = copyFrom.whole_;
        interval_ = copyFrom.interval_;
        savePoints_ = copyFrom.savePoints_;
        return *this;
    }

    Self &operator=(Self &&moveFrom)
    {
        whole_ = std::move(moveFrom.whole_);
        interval_ = moveFrom.interval_;
        savePoints_ = std::move(moveFrom.savePoints_);
        moveFrom.interval_.second = std::numeric_limits<size_t>::max();
        return *this;
    }

    bool Valid() const
    {
        return matchedInterval_.second <= whole_.Length();
    }

    operator bool() const
    {
        return Valid();
    }

    size_t operator[](size_t idx) const
    {
        AGZ_ASSERT(Valid() && idx < savePoints_.size());
        return savePoints_[idx];
    }

    size_t GetMatchedStart() const
    {
        AGZ_ASSERT(Valid());
        return matchedInterval_.first;
    }

    size_t GetMatchedEnd() const
    {
        AGZ_ASSERT(Valid());
        return matchedInterval_.second;
    }

    std::pair<size_t, size_t> GetMatchedInterval() const
    {
        AGZ_ASSERT(Valid());
        return interval_;
    }

    StringView<CS> operator()(size_t firstSavePoint, secondSavePoint) const
    {
        AGZ_ASSERT(Valid());
        AGZ_ASSERT(firstSavePoint <= secondSavePoint);
        AGZ_ASSERT(secondSavePoint <= savePoints_.size());
        return whole_.Slice(savePoints_[firstSavePoint],
                            savePoints_[secondSavePoint]);
    }

private:

    Match(const StringView<CS> &whole,
        const Interval &interval,
        std::vector<size_t> &&savePoints)
        : whole_(whole),
          interval_(interval),
          savePoints_(std::move(savePoints))
    {

    }

    String<CS> whole_;
    std::pair<size_t, size_t> matchedInterval_;
    std::vector<size_t> savePoints_;
};

template<typename CS, typename Eng = VMEngineImpl::VMEngine<CS>>
class Regex
{
public:

    using Charset   = CS;
    using CodePoint = typename CS::CodePoint;
    using CodeUnit  = typename CS::CodeUnit;
    using Engine    = Eng;
    using Self      = Regex<CS, Eng>;

private:

    Engine engine_;
};

AGZ_NS_END(AGZ)
