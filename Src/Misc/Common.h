#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

// ============================= forceinline =============================

#if defined(_MSC_VER)

#define AGZ_INLINE inline
#define AGZ_FORCEINLINE __forceinline

#elif defined __GNUC__
    #define AGZ_INLINE inline
    #define AGZ_FORCEINLINE inline __attribute__((always_inline))
#else
    #define AGZ_INLINE inline
    #define AGZ_FORCEINLINE inline
#endif

// ============================= namespace guard =============================

#define AGZ_NS_BEG(N) namespace N {
#define AGZ_NS_END(N) }

// ============================= assertion =============================

#include <cassert>
#define AGZ_ASSERT(X) assert(X)

// ============================= when __BYTE_ORDER__ unavailable =============================

#ifndef __BYTE_ORDER__
    #if defined(_MSC_VER)
        // Platform supporting MSVC must use little endian
        #define AGZ_LITTLE_ENDIAN
    #else
        // #define AGZ_BIG_ENDIAN
        #define AGZ_LITTLE_ENDIAN
    #endif
#endif

// ============================= operating system =============================

#if defined(_WIN32)
    #define AGZ_OS_WIN32
#elif defined(__linux)
    // IMPROVE
    #define AGZ_OS_LINUX
#endif

#if defined(_MSC_VER)
    #define AGZ_CC_MSVC
#elif defined(__clang__)
    #define AGZ_CC_CLANG
#elif defined(__GUNC__)
    #define AGZ_CC_GCC
#endif

// ============================= implementation flag =============================

#if defined(AGZ_ALL_IMPL)

#if not defined(AGZ_PLATFORM_IMPL)
#define AGZ_PLATFORM_IMPL
#endif

#if not defined(AGZ_TEXTURE_FILE_IMPL)
#define AGZ_TEXTURE_FILE_IMPL
#endif

#if not defined(AGZ_FILE_IMPL)
#define AGZ_FILE_IMPL
#endif

#endif

namespace AGZ {

// ============================= option =============================

template<typename T>
using Option = std::optional<T>;

template<typename T>
auto Some(T &&v) { return std::make_optional<T>(std::forward<T>(v)); }

constexpr std::nullopt_t None = std::nullopt;

// ============================= unreachable hint =============================

[[noreturn]] AGZ_FORCEINLINE void Unreachable()
{
#if defined(_MSC_VER)
    __assume(0);
#elif defined(__GUNC__)
    __builtin_unreachable();
#endif
    std::terminate();
}

// ============================= common deleter =============================

struct OperatorDeleter
{
    template<typename T>
    static void Delete(T *ptr) { ::operator delete(ptr); }
};

struct DummyDeleter
{
    template<typename T> static void Delete(T*) { }
};

// ============================= exception-safe array constructor =============================

template<typename C, typename Deleter, typename...Args>
void ConstructN(C *ptr, size_t n, const Args&...args) noexcept(noexcept(C(args...)))
{
    if constexpr(noexcept(C(args...)))
    {
        for(size_t i = 0; i < n; ++i)
            new(ptr + i) C(args...);
    }
    else
    {
        size_t i = 0;
        try
        {
            for(; i < n; ++i)
                new(ptr + i) C(args...);
        }
        catch(...)
        {
            for(size_t j = 0; j < i; ++j)
                (ptr + j)->~C();
            Deleter::Delete(ptr);
            throw;
        }
    }
}

// ============================= type flags (dummy arguments) =============================

struct Uninitialized_t { };
inline Uninitialized_t UNINITIALIZED;

template<typename T>
struct CanConvertToUninitializedFlag
{
    static constexpr bool value = std::is_convertible_v<T, Uninitialized_t>;
};

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

// ============================= Misc =============================

template<typename T>
using remove_rcv_t = std::remove_cv_t<std::remove_reference_t<T>>;

template<typename T>
constexpr T StaticMax(T lhs, T rhs) { return lhs < rhs ? rhs : lhs; }

inline size_t CombineHash(size_t fst, size_t snd)
{
    return fst ^ (snd + 0x9e3779b9 + (fst << 6) + (fst >> 2));
}

using std::size_t;

} // namespace AGZ
