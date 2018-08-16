#pragma once

#include <stdexcept>

#include "Config.h"

#ifdef _MSC_VER

#define AGZ_INLINE inline
#define AGZ_FORCEINLINE __forceinline

#elif defined __GNUC__

#define AGZ_INLINE inline
#define AZG_FORCEINLINE __attribute__((always_inline))

#else

#define AGZ_INLINE inline
#define AGZ_FORCEINLINE inline

#endif

#define AGZ_NS_BEG(N) namespace N {
#define AGZ_NS_END(N) }

#include <cassert>
#define AGZ_ASSERT(X) assert(X)

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

[[noreturn]]AGZ_FORCEINLINE void Unreachable()
{
#ifdef _MSC_VER
    __assume(0);
#elif defined(__GUNC__)
    __builtin_unreachable();
#else
    std::terminate();
#endif
}

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

class Exception : public std::runtime_error
{
public:
    explicit Exception(const std::string &err) : runtime_error(err) { }
};

class CharsetException : public Exception
{
public:
    explicit CharsetException(const std::string &err) : Exception(err) { }
};

class ArgumentException : public Exception
{
public:
    explicit ArgumentException(const std::string &err) : Exception(err) { }
};

template<typename T>
using remove_rcv_t = std::remove_cv_t<std::remove_reference_t<T>>;

template<typename T>
constexpr T StaticMax(T lhs, T rhs) { return lhs < rhs ? rhs : lhs; }

AGZ_NS_END(AGZ)
