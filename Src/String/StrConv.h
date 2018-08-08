#pragma once

#include <type_traits>

#include "../Misc/Common.h"
#include "String.h"

AGZ_NS_BEG(AGZ)

class StrConvException : public std::invalid_argument
{
public:
    explicit StrConvException(const std::string &err) : invalid_argument(err) { }
};

class Str2IntException : public StrConvException
{
public:
    explicit Str2IntException(const std::string &err) : StrConvException(err) { }
};

class StrConv
{
public:

    #define AGZ_CH template<typename CS> static String<CS> ToStr

    AGZ_CH(char c)               { return String<CS>(std::to_string(c)); }
    AGZ_CH(unsigned char c)      { return String<CS>(std::to_string(c)); }
    AGZ_CH(signed char c)        { return String<CS>(std::to_string(c)); }
    AGZ_CH(int c)                { return String<CS>(std::to_string(c)); }
    AGZ_CH(unsigned int c)       { return String<CS>(std::to_string(c)); }
    AGZ_CH(short c)              { return String<CS>(std::to_string(c)); }
    AGZ_CH(unsigned short c)     { return String<CS>(std::to_string(c)); }
    AGZ_CH(long c)               { return String<CS>(std::to_string(c)); }
    AGZ_CH(unsigned long c)      { return String<CS>(std::to_string(c)); }
    AGZ_CH(long long c)          { return String<CS>(std::to_string(c)); }
    AGZ_CH(unsigned long long c) { return String<CS>(std::to_string(c)); }
    AGZ_CH(float c)              { return String<CS>(std::to_string(c)); }
    AGZ_CH(double c)             { return String<CS>(std::to_string(c)); }
    AGZ_CH(const char *c,        NativeCharset cs = NativeCharset::UTF8) { return String<CS>(c, cs); }
    AGZ_CH(const std::string &s, NativeCharset cs = NativeCharset::UTF8) { return String<CS>(s, cs); }

    #undef AGZ_CH

    template<typename T, typename CS,
             std::enable_if_t<std::is_integral_v<T>, int> = 0>
    static T Parse(const typename String<CS>::View &view,
                   unsigned int base = 10);

    template<typename T, typename CS,
             std::enable_if_t<std::is_integral_v<T>, int> = 0>
    static T Parse(const String<CS> &str, unsigned int base = 10)
    {
        return Parse<T>(str.AsView(), base);
    }
};

AGZ_NS_BEG(StrImpl)

template<typename T, typename CS,
         std::enable_if_t<std::is_integral_v<T>, int> = 0>
struct Int2StrImpl
{
    T GetDigitValue(typename CS::CodeUnit cu, unsigned int base)
    {
        // IMPROVE: Consider looking-up table

        T ret;
        if('0' <= cu && cu <= '9')
            ret = static_cast<T>(cu - '0');
        else if('a' <= cu && cu <= 'z')
            ret = static_cast<T>(cu - 'a') + 10;
        else if('A' <= cu && cu <= 'Z')
            ret = static_cast<T>(cu - 'A') + 10;
        else
            ret = base;
        if(ret >= base)
            throw Str2IntException("Invalid digit");
        return ret;
    }

    T Conv(typename String<CS>::View s, unsigned int base)
    {
        // IMPROVE: This code runs unacceptably slow
        //          Consider DFA for acceleration

        if(base > 36)
            throw Str2IntException("Too large base");

        bool neg = false; size_t i = 0;
        if(s.Empty())
            throw Str2IntException("Empty string");
        if(s[0] == '+')
            s = s.Slice(1);
        else if(s[0] == '-')
        {
            if constexpr(!std::is_signed_v<T>)
                throw Str2IntException("Cannot be negative value");
            s = s.Slice(1);
            neg = true;
        }

        // Handle special leading chars
        if(s.StartsWith("0x") || s.StartsWith("0X"))
        {
            base = 16;
            s = s.Slice(2);
        }
        else if(s.StartsWith("0b") || s.StartsWith("0B"))
        {
            base = 2;
            s = s.Slice(2);
        }
        else if(s.StartsWith("0o") || s.StartsWith("0O"))
        {
            base = 8;
            s = s.Slice(2);
        }
        else if(s.StartsWith("0") && s.Length() > 1)
        {
            base = 8;
            s = s.Slice(1);
        }

        if(s.Empty())
            throw Str2IntException("Empty digit sequence");
        if(s[0] == '0')
        {
            if(s.Length() > 1)
                throw Str2IntException("More than one 0 in digit sequence");
            return 0;
        }

        T ret = 0; size_t len = s.Length();
        for(size_t i = 0; i < len; ++i)
            ret = base * ret + GetDigitValue(s[i], base);

        return ret;
    }
};

AGZ_NS_END(StrImpl)

template<typename T, typename CS,
         std::enable_if_t<std::is_integral_v<T>, int> = 0>
T StrConv::Parse<T, CS>(const typename String<CS>::View &view,
                        unsigned int base)
{
    return StrImpl::Int2StrImpl<T, CS>::Conv(view, base);
}

AGZ_NS_END(AGZ)
