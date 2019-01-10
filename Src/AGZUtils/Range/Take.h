#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

#include "../Misc/Common.h"
#include "Transform.h"

namespace AGZ {

namespace RangeAux
{
    template<typename R>
    class TakeImpl
    {
        R range_;
        GetIteratorType<R> end_;

    public:

        using Iterator = GetIteratorType<R>;

        TakeImpl(R range, size_t n)
            : range_(std::move(range))
        {
            end_ = AdvanceTo(std::begin(range_), std::end(range_),
                static_cast<typename std::iterator_traits<Iterator>
                                                ::difference_type>(n));
        }

        Iterator begin() const
        {
            return std::begin(range_);
        }

        Iterator end() const
        {
            return end_;
        }
    };

    template<typename R, typename F>
    class TakeWhileImpl
    {
        R range_;
        GetIteratorType<R> end_;

    public:

        using Iterator = GetIteratorType<R>;

        TakeWhileImpl(R &&range, F &&func)
            : range_(std::move(range))
        {
            Iterator beg = std::begin(range_), end = std::end(range_);
            while(beg != end && func(*beg))
                ++beg;
            end_ = beg;
        }

        Iterator begin() const
        {
            return std::begin(range_);
        }

        Iterator end() const
        {
            return end_;
        }
    };

    struct TakeTrait
    {
        template<typename R>
        using Impl = TakeImpl<R>;
    };

    template<typename F>
    struct TakeWhileTrait
    {
        template<typename R>
        using Impl = TakeWhileImpl<R, F>;
    };
}

/**
 * @brief 取某个range的前n个元素
 * 
 * @note 若原range仅拥有m < n个元素，则得到的range也只有m个元素
 */
inline auto Take(size_t n)
{
    return RangeAux::TransformWrapper<
        RangeAux::TakeTrait, size_t>(std::move(n));
}

/**
 * @brief 丢弃range中第一个不满足给定谓词的元素和它之后的元素
 */
template<typename F>
auto TakeWhile(F func)
{
    return RangeAux::TransformWrapper<
            RangeAux::TakeWhileTrait<F>,F>(std::move(func));
}

} // namespace AGZ
