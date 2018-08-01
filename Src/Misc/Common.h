#pragma once

#include <iterator>
#include <stdexcept>
#include <type_traits>

#ifdef _MSC_VER

#define AGZ_INLINE inline //__forceinline

#elif defined __GNUC__

#define AGZ_INLINE inline //__attribute__((always_inline))

#else

#define AGZ_INLINE inline

#endif

#ifdef _MSC_VER

#include <malloc.h>
#define AGZ_ALIGNED_ALLOC(align, size) _aligned_malloc((size), (align))
#define AGZ_ALIGNED_FREE(ptr) _aligned_free((ptr))

#else

#include <cstdlib>
#define AGZ_ALIGNED_ALLOC(align, size) (std::aligned_alloc)((align), (size))
#define AGZ_ALIGNED_FREE(ptr) (std::free)((ptr))

#endif

#define AGZ_NS_BEG(N) namespace N {
#define AGZ_NS_END(N) }

#include <cassert>
#define AGZ_ASSERT(X) assert(X)

#define AGZ_USE_SSE2

#ifndef __BYTE_ORDER__

#ifdef _MSC_VER

// Platform supporting MSVC must use little endian
#define AGZ_LITTLE_ENDIAN

#else

// #define AGZ_BIG_ENDIAN
#define AGZ_LITTLE_ENDIAN

#endif

#endif

AGZ_NS_BEG(AGZ)

struct Uninitialized_t { };
inline Uninitialized_t UNINITIALIZED;

struct CONS_FLAG_FROM_FN_t { };
inline CONS_FLAG_FROM_FN_t FROM_FN;

template<typename T>
struct CONS_FLAG_FROM_t { };
template<typename T>
inline CONS_FLAG_FROM_t<T> FROM;

struct CONS_FLAG_NOCHECK_t { };
inline CONS_FLAG_NOCHECK_t NOCHECK;

struct CONS_FLAG_UNSPECIFIED_t { };
inline CONS_FLAG_UNSPECIFIED_t UNSPECIFIED;

struct Void_t { };

class EncodingException : public std::invalid_argument
{
public:
    EncodingException(const std::string &err) : invalid_argument(err) { }
};

template<typename T>
using remove_rcv_t = std::remove_cv_t<std::remove_reference_t<T>>;

template<typename T>
constexpr bool IsRandomAccessIterator =
    std::is_base_of_v<std::random_access_iterator_tag,
                      typename T::iterator_category>;

template<typename T>
constexpr bool IsBidirectionalIterator =
    std::is_base_of_v<std::bidirectional_iterator_tag,
                      typename T::iterator_category>;

AGZ_NS_END(AGZ)
