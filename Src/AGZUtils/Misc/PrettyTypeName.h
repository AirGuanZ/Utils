#pragma once

#include <typeinfo>

#include "../Utils/String.h"
#include "Common.h"

namespace AGZ {

namespace PrettyTypeNameImpl
{
    template<typename T>
    struct Impl
    {
        static Str8 Name()
        {
            return Str8(typeid(T).name());
        }
    };

    template<typename T>
    struct Impl<const T>
    {
        static Str8 Name()
        {
            return "const " + Impl<T>::Name();
        }
    };

    template<typename T>
    struct Impl<T&>
    {
        static Str8 Name()
        {
            return "lref-to " + Impl<T>::Name();
        }
    };

    template<typename T>
    struct Impl<volatile T>
    {
        static Str8 Name()
        {
            return "vol " + Impl<T>::Name();
        }
    };

    template<typename T>
    struct Impl<T*>
    {
        static Str8 Name()
        {
            return "ptr-to " + Impl<T>::Name();
        }
    };

    template<typename T>
    struct Impl<T&&>
    {
        static Str8 Name()
        {
            return "rref-to " + Impl<T>::Name();
        }
    };

    template<bool Start>
    AGZ_FORCEINLINE Str8 GetArgListName()
    {
        return Str8();
    }

    template<bool Start, typename T, typename...Args>
    AGZ_FORCEINLINE Str8 GetArgListName()
    {
        if constexpr(Start)
            return Impl<T>::Name() + GetArgListName<false, Args...>();
        else
            return ", " + Impl<T>::Name() + GetArgListName<false, Args...>();
    }

    template<typename R, typename...Args>
    struct Impl<R(Args...)>
    {
        static Str8 Name()
        {
            StringBuilder<UTF8<>> b;
            b << Str8("(") << GetArgListName<true, Args...>()
              << Str8(") -> ") << Impl<R>::Name();
            return b.Get();
        }
    };

#define PREDEFINED_PRETTY_TYPENAME(TYPE, NAME) \
    template<> \
    struct Impl<TYPE> \
    { \
        static Str8 Name() \
        { \
            return #NAME; \
        } \
    }

PREDEFINED_PRETTY_TYPENAME(char, char);
PREDEFINED_PRETTY_TYPENAME(signed char, schar);
PREDEFINED_PRETTY_TYPENAME(unsigned char, uchar);
PREDEFINED_PRETTY_TYPENAME(short, short);
PREDEFINED_PRETTY_TYPENAME(unsigned short, ushort);
PREDEFINED_PRETTY_TYPENAME(int, int);
PREDEFINED_PRETTY_TYPENAME(unsigned int, uint);
PREDEFINED_PRETTY_TYPENAME(long, long);
PREDEFINED_PRETTY_TYPENAME(unsigned long, ulong);
PREDEFINED_PRETTY_TYPENAME(long long, llong);
PREDEFINED_PRETTY_TYPENAME(unsigned long long, ullong);

PREDEFINED_PRETTY_TYPENAME(float, float);
PREDEFINED_PRETTY_TYPENAME(double, double);
PREDEFINED_PRETTY_TYPENAME(long double, ldouble);

#undef PREDEFINED_PRETTY_TYPENAME
}

template<typename T>
Str8 PrettyTypeName()
{
    return PrettyTypeNameImpl::Impl<T>::Name();
}

} // namespace AGZ