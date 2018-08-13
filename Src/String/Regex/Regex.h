#pragma once

#include <vector>

#include "../../Misc/Common.h"
#include "../../Range/Iterator.h"
#include "../String.h"
#include "NFAVM.h"
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
    bool valid_;\
    String<CS> whole_;
    std::vector<typename String<CS>::View> segs_;

public:

    using Str      = String<CS>;
    using StrView  = StringView<CS>;
    using Self     = Match<CS>;
    using Iterator = GetIteratorType<decltype(segs_)>;

    Match()
        : valid_(false)
    {

    }

    Match(Str &&whole,
          const std::vector<std::pair<size_t, size_t>> &segs)
        : valid_(true), whole_(std::move(whole))
    {
        for(auto &seg : segs)
            segs_.push_back(whole_.Slice(seg.first, seg.second));
    }

    Match(const Self&) = delete;

    Match(Self &&moveFrom) noexcept
        : valid_(moveFrom.valid_),
          whole_(std::move(moveFrom.whole_)),
          segs_(std::move(moveFrom.segs_))
    {
        moveFrom.valid_ = false;
    }

    Self &operator=(const Self&) = delete;

    Self &operator=(Self &&moveFrom) noexcept
    {
        valid_  = moveFrom.valid_;
        whole_  = std::move(moveFrom.whole_);
        segs_   = std::move(moveFrom.segs_);
        moveFrom.valid_ = false;
        return *this;
    }

    operator bool() const
    {
        return Valid();
    }

    bool Valid() const
    {
        return valid_;
    }

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

template<typename CS, typename Eng = Pike::PikeMachine<CS>>
class Regex
{
    Eng engine_;

public:

    using Charset   = CS;
    using CodeUnit  = typename CS::CodeUnit;
    using CodePoint = typename CS::CodePoint;
    using Engine    = Eng;
    using Result    = Match<CS>;
    using Self      = Regex<CS, Eng>;

    explicit Regex(const StringView<CS> &expr)
        : engine_(expr)
    {

    }

    explicit Regex(const String<CS> &expr)
        : Regex(expr.AsView())
    {

    }

    Regex(const Self&)           = delete;
    Self &operator=(const Self&) = delete;
    ~Regex()                     = default;

    Result Match(const StringView<CS> &dst) const
    {
        auto [succ, subs] = engine_.Match(dst);
        if(!succ)
            return Result();
        return Result(std::move(dst), subs);
    }

    Result Match(const String<CS> &dst) const
    {
        return Match(dst.AsView());
    }
};

AGZ_NS_END(AGZ::RegexImpl)

AGZ_NS_BEG(AGZ)

template<typename CS>
using RegexMatch = RegexImpl::Match<CS>;

template<typename CS>
using Regex = RegexImpl::Regex<CS>;

using RegexMatch8 = RegexMatch<UTF8<>>;
using Regex8      = Regex<UTF8<>>;

AGZ_NS_END(AGZ)
