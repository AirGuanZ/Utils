#pragma once

#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>

#include "../../Misc/Common.h"

namespace AGZ::Str
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
 * @brief ����һ���ַ��������ؽ����е�����СдӢ����ĸת��Ϊ��д��ʽ��Ľ��
 */
template<typename TChar>
std::basic_string<TChar> ToUpper(const std::basic_string_view<TChar> &str)
{
    std::basic_string<TChar> ret(str);
    return ToUpperInPlace(ret);
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

/**
 * @brief ����һ���ַ��������ؽ����е����д�дӢ����ĸת��ΪСд��ʽ��Ľ��
 */
template<typename TChar>
std::basic_string<TChar> ToLower(const std::basic_string_view<TChar> &str)
{
    std::basic_string<TChar> ret(str);
    return ToLowerInPlace(ret);
}

/**
 * @brief ԭ��ɾ���ַ�����ͷ��������ָ��ν�ʵ��ַ�
 */
template<typename TChar, typename TPred>
std::basic_string<TChar> &TrimLeftInPlace(std::basic_string<TChar> &str, TPred &&pred)
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

/**
 * @brief ����һ���ַ���������ɾ������ͷ���������ָ��ν�ʵ��ַ���Ľ��
 */
template<typename TChar, typename TPred>
std::basic_string<TChar> TrimLeft(const std::basic_string_view<TChar> &str, TPred &&pred)
{
    std::basic_string<TChar> ret(str);
    return TrimLeftInPlace(ret, std::forward<TPred>(pred));
}

/**
 * @brief ԭ��ɾ���ַ�����ͷ���еĿհ��ַ�
 */
template<typename TChar>
std::basic_string<TChar> &TrimLeftInPlace(std::basic_string<TChar> &str)
{
    return TrimLeftInPlace(str, IsWhitespace<TChar>);
}

/**
 * @brief ����һ���ַ���������ɾ������ͷ���пհ��ַ���Ľ��
 */
template<typename TChar>
std::basic_string<TChar> TrimLeft(const std::basic_string_view<TChar> &str)
{
    std::pmr::basic_string<TChar> ret(str);
    return TrimLeftInPlace(ret);
}

/**
 * @brief ԭ��ɾ���ַ���ĩβ�����������ν�ʵ��ַ�
 */
template<typename TChar, typename TPred>
std::basic_string<TChar> &TrimRightInPlace(std::basic_string<TChar> &str, TPred &&pred)
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

/**
 * @brief ����һ���ַ���������ɾ����ĩβ��������ָ��ν�ʵ��ַ���Ľ��
 */
template<typename TChar, typename TPred>
std::basic_string<TChar> TrimRight(const std::basic_string_view<TChar> &str, TPred &&pred)
{
    std::basic_string<TChar> ret(str);
    return TrimRightInPlace(ret, std::forward<TPred>(pred));
}

/**
 * @brief ԭ��ɾ���ַ���ĩβ���пհ��ַ�
 */
template<typename TChar>
std::basic_string<TChar> &TrimRightInPlace(std::basic_string<TChar> &str)
{
    return TrimRightInPlace(str, IsWhitespace<TChar>);
}

/**
 * @brief ����һ���ַ���������ɾ����ĩβ���пհ��ַ���Ľ��
 */
template<typename TChar>
std::basic_string<TChar> TrimRight(const std::basic_string_view<TChar> &str)
{
    std::basic_string<TChar> ret(str);
    return TrimRightInPlace(ret);
}

/**
 * @brief ԭ��ɾ���ַ�����β�����������ν�ʵ��ַ�
 */
template<typename TChar, typename TPred>
std::basic_string<TChar> &TrimInPlace(std::basic_string<TChar> &str, TPred &&pred)
{
    return TrimRightInPlace(TrimLeftInPlace(str, pred), pred);
}

/**
 * @brief ����һ���ַ���������ɾ������β��������ָ��ν�ʵ��ַ���Ľ��
 */
template<typename TChar, typename TPred>
std::basic_string<TChar> Trim(const std::basic_string_view<TChar> &str, TPred &&pred)
{
    std::basic_string<TChar> ret(str);
    return TrimInPlace(ret, std::forward<TPred>(pred));
}

/**
 * @brief ԭ��ɾ���ַ�����β���пհ��ַ�
 */
template<typename TChar>
std::basic_string<TChar> &TrimInPlace(std::basic_string<TChar> &str)
{
    return TrimInPlace(str, IsWhitespace<TChar>);
}

/**
 * @brief ����һ���ַ���������ɾ������β���пհ��ַ���Ľ��
 */
template<typename TChar>
std::basic_string<TChar> Trim(const std::basic_string_view<TChar> &str)
{
    std::basic_string<TChar> ret(str);
    return TrimInPlace(ret);
}

/**
 * @brief ��һ���ַ�����һ���ַ���
 * @param joiner �������ӵ��ַ�
 * @param begin �����ӵ��ַ�����������ʼ������
 * @param end �����ӵ��ַ�����������ֹ������
 */
template<typename TChar, typename TIterator>
std::basic_string<TChar> Join(TChar &joiner, TIterator begin, TIterator end)
{
    if (begin == end)
        return std::basic_string<TChar>();
    std::basic_string<TChar> ret = *begin++;
    while (begin != end)
    {
        ret.push_back(joiner);
        ret.append(*begin++);
    }
    return ret;
}

/**
 * @brief ��һ���ַ�������һ���ַ���
 * @param joiner �������ӵ��ַ���
 * @param begin �����ӵ��ַ�����������ʼ������
 * @param end �����ӵ��ַ�����������ֹ������
 */
template<typename TChar, typename TIterator>
std::basic_string<TChar> Join(const std::basic_string_view<TChar> &joiner, TIterator begin, TIterator end)
{
    if(begin == end)
        return std::basic_string<TChar>();
    std::basic_string<TChar> ret = *begin++;
    while(begin != end)
    {
        ret.append(joiner);
        ret.append(*begin++);
    }
    return ret;
}

/**
 * @brief �ø������ַ�ν�ʷָ��ַ���
 * @param src ���ָ���ַ�����ɪɪ������
 * @param pred �ַ�ν�ʣ��������ν�ʵ��ַ�������Ϊ�ָ��ַ�
 * @param outIterator ��������������� std::basic_string_view<TChar> ���͵Ľ��
 * @param removeEmptyResult �Ƿ��Ƴ��ָ����еĿ��ַ�����ȱʡΪ true
 * @return �ָ��õ��˶��ٸ��Ӵ�
 */
template<typename TChar, typename TPred, typename TOutIterator>
size_t Split(
    const std::basic_string_view<TChar> &src, TPred &&pred,
    TOutIterator outIterator, bool removeEmptyResult = true)
{
    size_t beg = 0, ret = 0;
    while(beg < src.size())
    {
        size_t end = beg;
        while(end < src.size() && !pred(src[end]))
            ++end;
        if(end != beg || !removeEmptyResult)
        {
            ++ret;
            *outIterator++ = src.substr(beg, end - beg);
        }
        beg = end + 1;
    }
    return ret;
}

/**
 * @brief �ø������ַ����ָ��ַ���
 * @param src ���ָ���ַ�����ɪɪ������
 * @param splitter ������Ϊ�ָ���ַ���
 * @param outIterator ��������������� std::basic_string_view<TChar> ���͵Ľ��
 * @param removeEmptyResult �Ƿ��Ƴ��ָ����еĿ��ַ�����ȱʡΪ true
 * @return �ָ��õ��˶��ٸ��Ӵ�
 */
template<typename TChar, typename TOutIterator>
size_t Split(
    const std::basic_string_view<TChar> &src, const std::basic_string_view<TChar> &splitter,
    TOutIterator outIterator, bool removeEmptyResult = true)
{
    size_t beg = 0, ret = 0;
    while(beg < src.size())
    {
        size_t end = src.find(splitter, beg);
        if(end == std::basic_string_view<TChar>::npos)
        {
            ++ret;
            *outIterator++ = src.substr(beg, src.size() - beg);
            break;
        }
        if(end != beg || !removeEmptyResult)
        {
            ++ret;
            *outIterator++ = src.substr(beg, end - beg);
        }
        beg = end + splitter.size();
    }
    return ret;
}

/**
 * @brief �ÿհ��ַ���Ϊ�ָ����ָ��ַ���
 * @param src ���ָ���ַ�����ɪɪ������
 * @param outIterator ��������������� std::basic_string_view<TChar> ���͵Ľ��
 * @param removeEmptyResult �Ƿ��Ƴ��ָ����еĿ��ַ�����ȱʡΪ true
 * @return �ָ��õ��˶��ٸ��Ӵ�
 */
template<typename TChar, typename TOutIterator>
size_t Split(const std::basic_string_view<TChar> &src, TOutIterator outIterator, bool removeEmptyResult = true)
{
    return Split(src, IsWhitespace<TChar>, outIterator, removeEmptyResult);
}

/**
 * @brief ԭ�ؽ��ַ����е�ָ���Ӵ��滻Ϊ������Ӵ�
 * @param str �������ĸ��
 * @param oldSubstr ���滻�����Ӵ�
 * @param newSubstr �����滻�����Ӵ�
 * @return ���滻���˶��ٸ��Ӵ�
 */
template<typename TChar>
size_t ReplaceInPlace(
    std::basic_string<TChar> &str,
    const std::basic_string_view<TChar> &oldSubstr, const std::basic_string_view<TChar> &newSubstr)
{
    if(oldSubstr.empty())
        return 0;
    size_t ret = 0, pos;
    while((pos = str.find(oldSubstr, pos)) != std::basic_string_view<TChar>::npos)
    {
        str.replace(pos, oldSubstr.size(), newSubstr);
        pos += newSubstr.size();
        ++ret;
    }
    return ret;
}

/**
 * @brief ����һ���ַ��������ؽ����е�ָ���Ӵ��滻Ϊ������Ӵ���Ľ��
 * @param str �������ĸ��
 * @param oldSubstr ���滻�����Ӵ�
 * @param newSubstr �����滻�����Ӵ�
 * @return ���滻���˶��ٸ��Ӵ�
 */
template<typename TChar>
std::basic_string<TChar> Replace(
    const std::basic_string_view<TChar> &str,
    const std::basic_string_view<TChar> &oldSubstr, const std::basic_string_view<TChar> &newSubstr)
{
    std::basic_string<TChar> ret(str);
    ReplaceInPlace(ret, oldSubstr, newSubstr);
    return ret;
}

namespace Impl
{
    template<typename TChar, typename T, typename...Args> struct ToImpl { };
    template<> struct ToImpl<char, int8_t>   { static std::string Call(int8_t   obj) { return std::to_string(obj); } };
    template<> struct ToImpl<char, int16_t>  { static std::string Call(int16_t  obj) { return std::to_string(obj); } };
    template<> struct ToImpl<char, int32_t>  { static std::string Call(int32_t  obj) { return std::to_string(obj); } };
    template<> struct ToImpl<char, int64_t>  { static std::string Call(int64_t  obj) { return std::to_string(obj); } };
    template<> struct ToImpl<char, uint8_t>  { static std::string Call(uint8_t  obj) { return std::to_string(obj); } };
    template<> struct ToImpl<char, uint16_t> { static std::string Call(uint16_t obj) { return std::to_string(obj); } };
    template<> struct ToImpl<char, uint32_t> { static std::string Call(uint32_t obj) { return std::to_string(obj); } };
    template<> struct ToImpl<char, uint64_t> { static std::string Call(uint64_t obj) { return std::to_string(obj); } };
    template<> struct ToImpl<char, float>    { static std::string Call(float    obj) { return std::to_string(obj); } };
    template<> struct ToImpl<char, double>   { static std::string Call(double   obj) { return std::to_string(obj); } };

    template<typename TChar, typename T, typename...Args> struct FromImpl { };
    template<typename T> struct FromImpl<char, T>
    {
        static T Call(const std::string_view &str)
        {
            T ret;
            std::from_chars(str.data(), str.data() + str.size(), &ret);
            return ret;
        }
    };
} // namespace Impl

/**
 * @brief �������Ķ���ת��Ϊ�ַ���
 * @param obj ��ת���Ķ���
 * @param args ת������
 * @return ת���õ����ַ������
 */
template<typename TChar, typename T, typename...Args>
std::basic_string<TChar> To(T &&obj, Args&&...args)
{
    return Impl::ToImpl<TChar, remove_rcv_t<T>, remove_rcv_t<Args>...>::Call(std::forward<T>(obj), std::forward<Args>(args)...);
}

/**
 * @brief ���ַ�����parse��ָ�����͵Ķ���
 * @param src ��parse���ַ���
 * @param args parsing���̵Ĳ���
 * @return parsing�õ��Ľ��
 */
template<typename T, typename TChar, typename...Args>
T From(const std::basic_string_view<TChar> &src, Args&&...args)
{
    return Impl::FromImpl<TChar, remove_rcv_t<T>, remove_rcv_t<Args>...>::Call(src, std::forward<Args>(args)...);
}

} // namespace AGZ::Str
