#pragma once

#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "../../Misc/Common.h"
#include "../String/String.h"
#include "PikeVM.h"

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
class Match
{
public:

    using Interval = std::pair<size_t, size_t>;
    using Self = Match<CS>;

    Match()
        : interval_(0, std::numeric_limits<size_t>::max())
    {

    }

    Match(const StringView<CS> &whole,
          const Interval &interval,
          std::vector<size_t> &&savePoints)
        : whole_(whole),
          interval_(interval),
          savePoints_(std::move(savePoints))
    {

    }

    Match(const Self &copyFrom)
        : whole_(copyFrom.whole_),
          interval_(copyFrom.interval_),
          savePoints_(copyFrom.savePoints_)
    {

    }

    Match(Self &&moveFrom) noexcept
        : whole_(std::move(moveFrom.whole_)),
          interval_(moveFrom.interval_),
          savePoints_(std::move(moveFrom.savePoints_))
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

    Self &operator=(Self &&moveFrom) noexcept
    {
        whole_ = std::move(moveFrom.whole_);
        interval_ = moveFrom.interval_;
        savePoints_ = std::move(moveFrom.savePoints_);
        moveFrom.interval_.second = std::numeric_limits<size_t>::max();
        return *this;
    }

    bool Valid() const
    {
        return interval_.second <= whole_.Length();
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
        return interval_.first;
    }

    size_t GetMatchedEnd() const
    {
        AGZ_ASSERT(Valid());
        return interval_.second;
    }

    std::pair<size_t, size_t> GetMatchedInterval() const
    {
        AGZ_ASSERT(Valid());
        return interval_;
    }

    StringView<CS> operator()(size_t firstSavePoint, size_t secondSavePoint) const
    {
        AGZ_ASSERT(Valid());
        AGZ_ASSERT(firstSavePoint <= secondSavePoint);
        AGZ_ASSERT(secondSavePoint < savePoints_.size());
        return whole_.Slice(savePoints_[firstSavePoint],
                            savePoints_[secondSavePoint]);
    }

    size_t SavePointCount() const
    {
        return savePoints_.size();
    }

private:

    String<CS> whole_;
    std::pair<size_t, size_t> interval_;
    std::vector<size_t> savePoints_;
};

template<typename CS, typename Eng = PikeVM::Machine<CS>>
class Regex
{
public:

    using Charset   = CS;
    using CodePoint = typename CS::CodePoint;
    using CodeUnit  = typename CS::CodeUnit;
    using Engine    = Eng;
    using Result    = Match<CS>;
    using Self      = Regex<CS, Eng>;

    Regex() = default;

    Regex(const StringView<CS> &regex)
        : engine_(std::make_shared<Engine>(regex))
    {

    }

    Regex(const String<CS> &regex)
        : Regex(regex.AsView())
    {

    }

    Regex(const Self &copyFrom)
        : engine_(copyFrom.engine_)
    {

    }

    Regex(Self &&moveFrom) noexcept
        : engine_(std::move(moveFrom.engine_))
    {

    }

    Self &operator=(const Self &copyFrom)
    {
        engine_ = copyFrom.engine_;
        return *this;
    }

    Self &operator=(Self &&moveFrom) noexcept
    {
        engine_ = std::move(moveFrom.engine_);
        return *this;
    }

    ~Regex() = default;

    AGZ_FORCEINLINE Result Match(const String<CS> &dst) const
    {
        return this->Match(dst.AsView());
    }

    AGZ_FORCEINLINE Result Search(const String<CS> &dst) const
    {
        return this->Search(dst.AsView());
    }

    Result Match(const StringView<CS> &dst) const
    {
        auto rt = engine_->Match(dst);
        if(!rt.has_value())
            return Result();
        return Result(dst, { 0, dst.Length() }, std::move(rt.value()));
    }

    Result Search(const StringView<CS> &dst) const
    {
        auto rt = engine_->Search(dst);
        if(!rt.has_value())
            return Result();
        return Result(dst, rt.value().first, std::move(rt.value().second));
    }

private:

    std::shared_ptr<Engine> engine_;
};

using Regex8  = Regex<UTF8<>>;
using Regex16 = Regex<UTF16<>>;
using Regex32 = Regex<UTF32<>>;
using WRegex  = Regex<WUTF>;

AGZ_NS_END(AGZ)
