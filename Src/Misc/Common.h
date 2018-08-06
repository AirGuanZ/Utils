#pragma once

#include <stdexcept>

#ifdef _MSC_VER

#define AGZ_INLINE inline //__forceinline
#define AGZ_FORCEINLINE __forceinline

#elif defined __GNUC__

#define AGZ_INLINE inline //__attribute__((always_inline))
#define AZG_FORCEINLINE __attribute__((always_inline))

#else

#define AGZ_INLINE inline
#define AGZ_FORCEINLINE inline

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
    explicit EncodingException(const std::string &err) : invalid_argument(err) { }
};

template<typename T>
using remove_rcv_t = std::remove_cv_t<std::remove_reference_t<T>>;

AGZ_NS_END(AGZ)
