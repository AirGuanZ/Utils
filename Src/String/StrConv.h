#pragma once

#include "../Misc/Common.h"
#include "String.h"

AGZ_NS_BEG(AGZ)

class StrConvException : public std::invalid_argument
{
public:
    explicit StrConvException(const std::string &err) : invalid_argument(err) { }
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

    template<typename T, typename CS>
    static T Parse(const typename String<CS>::View &view);
    template<typename T, typename CS>
    static T Parse(const String<CS> &str) { return Parse(str.AsView()); }
};

AGZ_NS_END(AGZ)
