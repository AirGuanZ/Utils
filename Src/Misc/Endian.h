#pragma once

#include "../Misc/Common.h"

namespace AGZ::Endian {

enum class Endian { Big, Little };

#if defined(__BYTE_ORDER__)
constexpr bool IS_LITTLE_ENDIAN = __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
#elif defined(AGZ_BIG_ENDIAN)
constexpr bool IS_LITTLE_ENDIAN = false;
#else
constexpr bool IS_LITTLE_ENDIAN = true;
#endif

constexpr bool IS_BIG_ENDIAN = !IS_LITTLE_ENDIAN;

constexpr Endian ENDIAN = IS_BIG_ENDIAN ? Endian::Big : Endian::Little;

namespace Impl
{
    template<typename T, size_t S>
    struct Big2LittleImpl { };

    template<typename T>
    struct Big2LittleImpl<T, 1>
    {
        static T Convert(T val)
        {
            return val;
        }
    };

    template<typename T>
    struct Big2LittleImpl<T, 2>
    {
        static T Convert(T val)
        {
            T ret;
            char *p = reinterpret_cast<char*>(&ret);
            char *q = reinterpret_cast<char*>(&val);
            p[0] = q[1];
            p[1] = q[0];
            return ret;
        }
    };

    template<typename T>
    struct Big2LittleImpl<T, 4>
    {
        static T Convert(T val)
        {
            T ret;
            char *p = reinterpret_cast<char*>(&ret);
            char *q = reinterpret_cast<char*>(&val);
            p[0] = q[3];
            p[1] = q[2];
            p[2] = q[1];
            p[3] = q[0];
            return ret;
        }
    };

    template<typename T>
    struct Big2LittleImpl<T, 8>
    {
        static T Convert(T val)
        {
            T ret;
            char *p = reinterpret_cast<char*>(&ret);
            char *q = reinterpret_cast<char*>(&val);
            p[0] = q[7];
            p[1] = q[6];
            p[2] = q[5];
            p[3] = q[4];
            p[4] = q[3];
            p[5] = q[2];
            p[6] = q[1];
            p[7] = q[0];
            return ret;
        }
    };

    template<typename T, size_t S>
    using Little2BigImpl = Big2LittleImpl<T, S>;
}

template<typename T>
T Big2Little(T val)
{
    return Impl::Big2LittleImpl<T, sizeof(T)>::Convert(val);
}

template<typename T>
T Little2Big(T val)
{
    return Impl::Little2BigImpl<T, sizeof(T)>::Convert(val);
}

template<typename T>
T Native2Big(T val)
{
    if constexpr(IS_BIG_ENDIAN)
        return val;
    else
        return Little2Big<T>(val);
}

template<typename T>
T Native2Little(T val)
{
    if constexpr(IS_LITTLE_ENDIAN)
        return val;
    else
        return Big2Little<T>(val);
}

} // namespace AGZ::Endian
