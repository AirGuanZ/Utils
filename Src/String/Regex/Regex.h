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

namespace AGZ {

/**
 * @brief 正则表达式匹配/搜索结果
 */
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

	/**
	 * @brief 匹配/搜索操作是否成功
	 * 
	 * @note 只有该函数结果为true时，其他的内容查询操作才合法
	 */
    bool Valid() const
    {
        return interval_.second <= whole_.Length();
    }

	//! @copydoc Match<CS>::Valid
    operator bool() const
    {
        return Valid();
    }

	//! 取得第idx个保存点所记录的码元位置
    size_t operator[](size_t idx) const
    {
        AGZ_ASSERT(Valid() && idx < savePoints_.size());
        return savePoints_[idx];
    }

	//! 搜索得到的子串的第一个码元的位置
    size_t GetMatchedStart() const
    {
        AGZ_ASSERT(Valid());
        return interval_.first;
    }

	//! 搜索得到的子串的结尾的下一个码元位置
    size_t GetMatchedEnd() const
    {
        AGZ_ASSERT(Valid());
        return interval_.second;
    }

	//! 搜索得到的子串的码元下标范围
    std::pair<size_t, size_t> GetMatchedInterval() const
    {
        AGZ_ASSERT(Valid());
        return interval_;
    }

	//! 取得一对保存点所记录的位置间的子串
    StringView<CS> operator()(size_t firstSavePoint, size_t secondSavePoint) const
    {
        AGZ_ASSERT(Valid());
        AGZ_ASSERT(firstSavePoint <= secondSavePoint);
        AGZ_ASSERT(secondSavePoint < savePoints_.size());
        return whole_.Slice(savePoints_[firstSavePoint],
                            savePoints_[secondSavePoint]);
    }

	//! 共定义了多少个保存点
    size_t SavePointCount() const
    {
        return savePoints_.size();
    }

private:

    String<CS> whole_;
    std::pair<size_t, size_t> interval_;
    std::vector<size_t> savePoints_;
};

/**
 * @brief 正则表达式类，表达式语法与所用的引擎有关，缺省使用PikeVM引擎
 */
template<typename CS, typename Eng = StrImpl::PikeVM::Machine<CS>>
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

	//! 用该表达式匹配整个目标字符串
    AGZ_FORCEINLINE Result Match(const String<CS> &dst) const
    {
        return this->Match(dst.AsView());
    }

	//! 在目标串中搜寻能和该表达式匹配的子串
    AGZ_FORCEINLINE Result Search(const String<CS> &dst) const
    {
        return this->Search(dst.AsView());
    }

	//! 用该表达式匹配整个目标字符串
    Result Match(const StringView<CS> &dst) const
    {
        auto rt = engine_->Match(dst);
        if(!rt.has_value())
            return Result();
        return Result(dst, { 0, dst.Length() }, std::move(rt.value()));
    }

	//! 在目标串中搜寻能和该表达式匹配的子串
    Result Search(const StringView<CS> &dst) const
    {
        auto rt = engine_->Search(dst);
        if(!rt.has_value())
            return Result();
        return Result(dst, rt.value().first, std::move(rt.value().second));
    }

	//! 搜索能匹配该表达式的目标串前缀
    Result SearchPrefix(const StringView<CS> &dst) const
    {
        auto rt = engine_->SearchPrefix(dst);
        if(!rt.has_value())
            return Result();
        return Result(dst, rt.value().first, std::move(rt.value().second));
    }

	//! 搜索能匹配该表达式的目标串后缀
    Result SearchSuffix(const StringView<CS> &dst) const
    {
        auto rt = engine_->SearchSuffix(dst);
        if(!rt.has_value())
            return Result();
        return Result(dst, rt.value().first, std::move(rt.value().second));
    }

private:

    std::shared_ptr<Engine> engine_;
};

using Regex8  = Regex<UTF8<>>;  ///< 用于UTF-8编码字符串的正则表达式
using Regex16 = Regex<UTF16<>>; ///< 用于UTF-16编码字符串的正则表达式
using Regex32 = Regex<UTF32<>>; ///< 用于UTF-32编码字符串的正则表达式
using WRegex  = Regex<WUTF>;    ///< 用于宽字符编码字符串的正则表达式

} // namespace AGZ
