#pragma once

#include <string>
#include <string_view>

namespace AGZ::StdStr
{

/**
 * @brief �ж�һ���ַ��Ƿ��ǿհ��ַ�
 */
template<typename TChar>
bool IsWhitespace(TChar ch) noexcept
{
    return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v' || ch == '\f';
}

/**
 * @brief �ж�һ���ַ��Ƿ���СдӢ����ĸ
 */
template<typename TChar>
bool IsLower(TChar ch) noexcept
{
    return 'a' <= ch && ch <= 'z';
}

/**
 * @brief �ж�һ���ַ��Ƿ��Ǵ�дӢ����ĸ
 */
template<typename TChar>
bool IsUpper(TChar ch) noexcept
{
    return 'A' <= ch && ch <= 'Z';
}

/**
 * @brief �ж�һ���ַ��Ƿ���Ӣ����ĸ
 */
template<typename TChar>
bool IsAlpha(TChar ch) noexcept
{
    return IsLower(ch) || IsUpper(ch);
}

/**
 * @brief �ж�һ���ַ��Ƿ���ʮ��������
 */
template<typename TChar>
bool IsDemDigit(TChar ch) noexcept
{
    return '0' <= ch && ch <= '9';
}

/**
 * @brief ��Сд��ĸת��Ϊ��д��ĸ�������벻��Сд��ĸ����ԭ������
 */
template<typename TChar>
TChar ToUpper(TChar ch) noexcept
{
    return IsLower(ch) ? (ch - 'a' + 'A') : ch;
}

/**
 * @brief ����д��ĸת��ΪСд��ĸ�������벻�Ǵ�д��ĸ����ԭ������
 */
template<typename TChar>
TChar ToLower(TChar ch) noexcept
{
    return IsUpper(ch) ? (ch - 'A' + 'a') : ch;
}

/**
 * @brief ���ַ����е�СдӢ����ĸԭ��ת��Ϊ��д��ʽ
 */
template<typename TChar>
std::basic_string<TChar> &ToUpperInPlace(std::basic_string<TChar> &str) noexcept
{
    for(auto &ch : str)
        ch = ToUpper(ch);
    return str;
}

/**
 * @brief ���ַ����еĴ�дӢ����ĸԭ��ת��ΪСд��ʽ
 */
template<typename TChar>
std::basic_string<TChar> &ToLowerInPlace(std::basic_string<TChar> &str) noexcept
{
    for(auto &ch : str)
        ch = ToLower(ch);
    return str;
}

template<typename TChar, typename Pred>
std::basic_string<TChar> &TrimLeftInPlace(std::basic_string<TChar> &str, Pred &&pred)
{
    if(str.empty())
        return str;
    auto it = str.begin();
    while(it != str.end())
    {
        if(!pred(*it))
            break;
        ++it;
    }
    str.erase(str.begin(), it);
    return str;
}

template<typename TChar>
std::basic_string<TChar> &TrimLeftInPlace(std::basic_string<TChar> &str)
{
    return TrimLeftInPlace(str, IsWhitespace<TChar>);
}

template<typename TChar, typename Pred>
std::basic_string<TChar> &TrimRightInPlace(std::basic_string<TChar> &str, Pred &&pred)
{
    if(str.empty())
        return str;
    auto it = str.end();
    do
    {
        if(!pred(*(it - 1)))
            break;
        --it;
    } while(it != str.begin());
    str.erase(it, str.end());
    return str;
}

template<typename TChar>
std::basic_string<TChar> &TrimRightInPlace(std::basic_string<TChar> &str)
{
    return TrimRightInPlace(str, IsWhitespace<TChar>);
}

template<typename TChar, typename Pred>
std::basic_string<TChar> &TrimInPlace(std::basic_string<TChar> &str, Pred &&pred)
{
    return TrimRightInPlace(TrimLeftInPlace(str, pred), pred);
}

template<typename TChar, typename Pred>
std::basic_string<TChar> TrimLeft(const std::basic_string_view<TChar> &str, Pred &&pred)
{
    std::basic_string<TChar> ret(str);
    return TrimLeftInPlace(ret, std::forward<Pred>(pred));
}

} // namespace AGZ::StdStr
