#pragma once

#include "../Misc/Common.h"
#include "Iterator.h"
#include "Transform.h"

namespace AGZ {

namespace RangeAux
{
    template<typename R>
    class DropImpl
    {
        R range_;
        GetIteratorType<R> beg_;

    public:

        using Iterator = GetIteratorType<R>;

        DropImpl(R &&range, size_t n)
            : range_(std::move(range))
        {
            beg_ = AdvanceTo(std::begin(range_), std::end(range_),
                        static_cast<typename std::iterator_traits<
                            GetIteratorType<R>>::difference_type>(n));
        }

        Iterator begin() const { return beg_; }
        Iterator end() const { return std::end(range_); }
    };

    template<typename R, typename F>
    class DropWhileImpl
    {
        R range_;
        GetIteratorType<R> beg_;

    public:

        using Iterator = GetIteratorType<R>;

        DropWhileImpl(R &&range, F &&func)
            : range_(std::move(range))
        {
            Iterator end = std::end(range_);
            beg_ = std::begin(range_);
            while(beg_ != end && func(*beg_))
                ++beg_;
        }

        Iterator begin() const { return beg_; }
        Iterator end() const { return std::end(range_); }
    };

    struct DropTrait
    {
        template<typename R>
        using Impl = DropImpl<R>;
    };

    template<typename F>
    struct DropWhileTrait
    {
        template<typename R>
        using Impl = DropWhileImpl<R, F>;
    };
}

/**
 * @brief 丢弃某range中的前n个元素
 * 
 * @note 若range中元素不足n个，则会得到一个空range
 */
inline auto Drop(size_t n)
{
    return RangeAux::TransformWrapper<
            RangeAux::DropTrait, size_t>(std::move(n));
}

/**
 * @brief 不断丢弃range中的元素，直到遇到第一个不满足谓词的元素为止
 */
template<typename F>
auto DropWhile(F &&func)
{
    return RangeAux::TransformWrapper<
            RangeAux::DropWhileTrait<F>, F>(
                    std::forward<F>(func));
}

} // namespace AGZ
