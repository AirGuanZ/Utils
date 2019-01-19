#pragma once

#include <tuple>

#include "../String/String.h"
#include "../String/StrAlgo.h"

namespace AGZ
{

AGZ_NEW_EXCEPTION(FormatStringException);
 
namespace FMTImpl
{
    template<typename CS, typename T>
    String<CS> ConvertSingleArgumentToString(const T &arg)
    {
        return ToString<CS>(arg);
    }

    template<typename CS>
    String<CS> FetchArgString(int)
    {
        throw FormatStringException("Invalid format statement");
    }

    template<typename CS, typename FirstArg, typename...OtherArgs>
    String<CS> FetchArgString(int i, const FirstArg &firstArg, const OtherArgs&...otherArgs)
    {
        if(!i)
            return ToString<CS>(firstArg);
        return FetchArgString<CS>(i - 1, otherArgs...);
    }
} // namespace FMTImpl

/**
 * @brief 字符串格式化函数
 * 使用“{}”来引用参数，引用下标从0开始；参数将通过AGZ::ToString(arg)来转换为字符串。
 * 可以在“{}”中显示给定所引用的参数下标。默认下标总是上一个引用所使用的下标值加1。
 * @exception FormatStringException 格式字符串非法或参数无法转换为字符串时抛出
 */
template<typename CS, typename...Args>
String<CS> TFormat(const Str8 &fmt, const Args&...args)
{
    auto cpr = fmt.CodePoints();
    StringBuilder<CS> builder;
    int defaultArgIdx = 0;
    for(auto it = cpr.begin(), end = cpr.end(); it != end; ++it)
    {
        char32_t c = *it;
        if(c == '{')
        {
            ++it;
            if(it == end)
                throw FormatStringException("Invalid format string: '}' expected");
            auto fc = *it;
            if(fc == '{')
                builder.Append("{");
            else if(fc == '}')
                builder.Append(FMTImpl::FetchArgString<CS>(defaultArgIdx++, args...));
            else if(StrAlgo::IsUnicodeDigit(fc))
            {
                int i = fc - '0';
                for(;;)
                {
                    if(++it == end)
                        throw FormatStringException("Invalid format string: } expected");
                    auto d = *it;
                    if(StrAlgo::IsUnicodeDigit(d))
                        i = 10 * i + (d - '0');
                    else if(d == '}')
                        break;
                    else
                        throw FormatStringException("Invalid format string: invalid argument reference");
                }
                defaultArgIdx = i + 1;
                builder.Append(FMTImpl::FetchArgString<CS>(i, args...));
            }
        }
        else
            builder.Append(String<CS>(Str32(c, 1)));
    }
    return builder.Get();
}

template<typename...Args> Str8  Fmt8 (const Args&...args) { return TFormat<UTF8<>> (args...); }
template<typename...Args> Str16 Fmt16(const Args&...args) { return TFormat<UTF16<>>(args...); }
template<typename...Args> Str32 Fmt32(const Args&...args) { return TFormat<UTF32<>>(args...); }
template<typename...Args> AStr  AFmt (const Args&...args) { return TFormat<ASCII<>>(args...); }
template<typename...Args> WStr  WFmt (const Args&...args) { return TFormat<WUTF>   (args...); }

} // namespace AGZ
