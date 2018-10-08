#pragma once

#include <stdexcept>

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

#define AGZ_NS_BEG(N) namespace N {
#define AGZ_NS_END(N) }

#include <cassert>
#define AGZ_ASSERT(X) assert(X)

#ifndef __BYTE_ORDER__
    #if defined(_MSC_VER)
        // Platform supporting MSVC must use little endian
        #define AGZ_LITTLE_ENDIAN
    #else
        // #define AGZ_BIG_ENDIAN
        #define AGZ_LITTLE_ENDIAN
    #endif
#endif

#if defined(_WIN32)
    #define AGZ_OS_WIN32
#elif defined(__linux)
    // IMPROVE
    #define AGZ_OS_LINUX
#endif

#if defined(AGZ_ALL_IMPL)

#if not defined(AGZ_PLATFORM_IMPL)
#define AGZ_PLATFORM_IMPL
#endif

#if not defined(AGZ_TEXTURE_FILE_IMPL)
#define AGZ_TEXTURE_FILE_IMPL
#endif

#endif

AGZ_NS_BEG(AGZ)

[[noreturn]] AGZ_FORCEINLINE void Unreachable()
{
#if defined(_MSC_VER)
    __assume(0);
#elif defined(__GUNC__)
    __builtin_unreachable();
#else
    std::terminate();
#endif
}

struct OperatorDeleter
{
    template<typename T>
    static void Delete(T *ptr) { ::operator delete(ptr); }
};

struct DummyDeleter
{
    template<typename T> static void Delete(T*) { }
};

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

class OSException : public Exception
{
public:
    explicit OSException(const std::string &err) : Exception(err) { }
};

class FileException : public Exception
{
public:
    explicit FileException(const std::string &err) : Exception(err) { }
};

class UnreachableException : public Exception
{
public:
    explicit UnreachableException(const std::string &err) : Exception(err) { }
};

template<typename T>
using remove_rcv_t = std::remove_cv_t<std::remove_reference_t<T>>;

template<typename T>
constexpr T StaticMax(T lhs, T rhs) { return lhs < rhs ? rhs : lhs; }

AGZ_NS_END(AGZ)
