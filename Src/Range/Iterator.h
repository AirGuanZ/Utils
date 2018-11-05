#pragma once

#include <algorithm>
#include <iterator>
#include <type_traits>

#include "../Misc/Common.h"

namespace AGZ {

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

template<typename R>
typename R::Iterator GetIteratorImpl();
template<typename R>
typename R::const_iterator GetIteratorImpl();

template<typename R>
using GetIteratorType = decltype(GetIteratorImpl<R>());

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

} // namespace AGZ
