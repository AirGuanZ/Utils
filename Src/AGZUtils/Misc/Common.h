#pragma once

#include <cstdint>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

// 功能宏

// AGZ_USE_SSE2
// AGZ_USE_AVX
// AGZ_THREAD_SAFE_STRING
// AGZ_ENABLE_STRING_SSO
// AGZ_USE_GLFW
// AGZ_USE_OPENGL

#if defined(_DEBUG) || defined(DEBUG)
#include <cassert>
#define AGZ_ASSERT(X) assert(X)
#define AGZ_IN_DEBUG
#else
#define AGZ_ASSERT(X)
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
#elif defined(__GNUC__)
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

#if not defined(AGZ_GLFWCAPTURER_IMPL)
#define AGZ_GLFWCAPTURER_IMPL
#endif

#endif

namespace AGZ {

// ============================= unreachable hint =============================

[[noreturn]] inline void Unreachable()
{
#if defined(_MSC_VER)
    __assume(0);
#elif defined(__GNUC__)
    __builtin_unreachable();
#endif
    std::terminate();
}

/**
 * @brief 用一组可调用对象构造一个variant visitor，并用以匹配一个std::variant对象
 * @param e 被匹配的variant对象
 * @param vs 用于处理各variant分支的functor class实例。
 *			  它们的operator()返回值类型必须全部相同，且参数能覆盖所有的variant情形。
 *			  可以用[](auto){ ... }来处理默认分支，类似许多语言的模式匹配中的“_”。
 */
template<typename E, typename...Vs>
auto MatchVariant(E &&e, Vs...vs)
{
    struct overloaded : public Vs...
    {
        explicit overloaded(Vs...vss) : Vs(vss)... { }
        using Vs::operator()...;
    };
    return std::visit(overloaded(vs...), std::forward<E>(e));
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
void ConstructN(C *ptr, std::size_t n, const Args&...args) noexcept(noexcept(C(args...)))
{
    if constexpr(noexcept(C(args...)))
    {
        for(std::size_t i = 0; i < n; ++i)
            new(ptr + i) C(args...);
    }
    else
    {
        std::size_t i = 0;
        try
        {
            for(; i < n; ++i)
                new(ptr + i) C(args...);
        }
        catch(...)
        {
            for(std::size_t j = 0; j < i; ++j)
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

inline std::size_t CombineHash(std::size_t fst, std::size_t snd)
{
    return fst ^ (snd + 0x9e3779b9 + (fst << 6) + (fst >> 2));
}

using std::size_t;

template<typename T, int N> constexpr size_t ArraySize([[maybe_unused]] T (&arr)[N]) { return N; }

template<typename C, typename M>
ptrdiff_t ByteOffsetOf(M(C::*memPtr)) noexcept
{
    // 理论上这是UB，但我不知道有什么更好的方法能从成员指针得到成员在实例中的偏移量
    return reinterpret_cast<char*>(&(reinterpret_cast<C*>(0)->*memPtr))
         - reinterpret_cast<char*>(  reinterpret_cast<C*>(0));
}

/**
 * @brief 不可复制类模板
 */
class Uncopiable
{
public:

    Uncopiable() = default;
    Uncopiable(const Uncopiable&) = delete;
    Uncopiable &operator=(const Uncopiable&) = delete;
    Uncopiable(Uncopiable&&) noexcept = default;
    Uncopiable &operator=(Uncopiable&&) noexcept = default;
};

/**
 * @brief 不可移动类模板
 */
class Unmovable
{
public:

    Unmovable() = default;
    Unmovable(Unmovable&&) noexcept = delete;
    Unmovable &operator=(Unmovable&&) noexcept = delete;
};

} // namespace AGZ

namespace agz = AGZ;
