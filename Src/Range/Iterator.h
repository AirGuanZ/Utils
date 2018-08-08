#pragma once

#include <iterator>
#include <type_traits>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

template<typename T>
constexpr bool IsRandomAccessIterator =
    std::is_base_of_v<std::random_access_iterator_tag,
                      typename std::iterator_traits<T>::iterator_category>;

template<typename T>
constexpr bool IsBidirectionalIterator =
    std::is_base_of_v<std::bidirectional_iterator_tag,
                      typename std::iterator_traits<T>::iterator_category>;

template<typename T>
constexpr bool IsForwardIterator =
    std::is_base_of_v<std::forward_iterator_tag,
                      typename std::iterator_traits<T>::iterator_category>;

template<typename I, std::enable_if_t<IsRandomAccessIterator<I>, int> = 0>
I AdvanceTo(const I &cur, const I &end,
            typename std::iterator_traits<I>::difference_type n)
{
    AGZ_ASSERT(n >= 0);
    return cur + std::min(end - cur, n);
}

template<typename I, std::enable_if_t<(!IsRandomAccessIterator<I> &&
                                       IsForwardIterator<I>), int> = 0>
I AdvanceTo(const I &cur, const I &end,
            typename std::iterator_traits<I>::difference_type n)
{
    AGZ_ASSERT(n >= 0);
    I ret = cur;
    while(ret != end && n-- > 0)
        ++ret;
    return ret;
}

template<typename R, typename T = void>
struct GetIteratorImpl { };
template<typename R>
struct GetIteratorImpl<R, std::void_t<typename R::Iterator>>
{
    using Type = typename R::Iterator;
};
template<typename R>
struct GetIteratorImpl<R, std::void_t<typename R::const_iterator>>
{
    using Type = typename R::const_iterator;
};

template<typename R>
using GetIteratorType = typename GetIteratorImpl<R>::Type;

template<typename T>
class ValuePointer
{
    T val_;

public:

    explicit ValuePointer(T val)
        : val_(std::move(val))
    {

    }

    const T &operator*() const
    {
        return val_;
    }

    T &operator*()
    {
        return val_;
    }

    const T *operator->() const
    {
        return &val_;
    }

    T *operator->()
    {
        return &val_;
    }
};

AGZ_NS_END(AGZ)
