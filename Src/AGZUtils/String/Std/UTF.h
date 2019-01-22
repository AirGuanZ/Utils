#pragma once

#include <tuple>

#include "../../Misc/Common.h"
#include "../../Misc/Exception.h"

namespace AGZ::Str
{

AGZ_NEW_EXCEPTION(UTFException);

/**
 * @brief UTF-8 的code point、code unit转换
 * @tparam TCodeUnit 用于表示一个CodeUnit的类型，大小应为1
 * @tparam TCodePoint 用于表示一个CodePoint的类型，大小应为4
 * @tparam TUseException 是否在出现编码错误时使用异常，缺省为 true
 */
template<typename TCodeUnit, typename TCodePoint, bool TUseException = true>
class TUTF8
{
    static_assert(sizeof(TCodeUnit) == 1, "sizeof TCodeUnit of UTF-8 must be 1");
    static_assert(sizeof(TCodePoint) == 4, "sizeof TCodePoint of UTF-8 must be 4");

public:

    using CodePoint = TCodePoint;
    using CodeUnit  = TCodeUnit;

    /**
     * @brief 将一个CodePoint转换为CodeUnit时最多需要多少个CodeUnit的空间
     */
    static constexpr size_t MAX_CU_COUNT_IN_ONE_CP = 4;

    /**
     * @brief 从一系列CodeUnit的开头提取一个CodePoint
     * @param cu CodeUnit串的指针
     * @return 二元组(CP, pCU)，其中CP是提取出的CodePoint，pCU是剩余的CodeUnit串的首元素指针
     *         若没有启用异常，则当出现不合法CodeUnit串时返回(0, nullptr)
     * @exception TUseException 若启用了异常，则在遇到不合法CodeUnit串时抛出
     */
    static std::pair<CodePoint, const CodeUnit*> Decode(const CodeUnit *cu) noexcept(!TUseException)
    {
        AGZ_ASSERT(cu);

        CodeUnit fst = *cu++;

        // 1 bytes
        if(!(fst & 0b10000000))
            return { static_cast<CodePoint>(fst), cu };

#define NEXT(C, DST) \
    do {\
        CodeUnit ch = (C); \
        if((ch & 0b11000000) != 0b10000000) \
        { \
            if constexpr(TUseException) \
                throw UTFException("Decoding invalid utf-8 sequence"); \
            else \
                return { 0, nullptr }; \
        } \
        (DST) = ch & 0b00111111; \
    } while(0)

        // 2 bytes
        if((fst & 0b11100000) == 0b11000000)
        {
            CodePoint low;
            NEXT(*cu++, low);
            return { ((fst & 0b00011111) << 6) | low, cu };
        }

        // 3 bytes
        if((fst & 0b11110000) == 0b11100000)
        {
            CodePoint high, low;
            NEXT(*cu++, high);
            NEXT(*cu++, low);
            return { ((fst & 0b00001111) << 12) | (high << 6) | low, cu };
        }

        // 4 bytes
        if((fst & 0b11111000) == 0b11110000)
        {
            CodePoint high, medi, low;
            NEXT(*cu++, high);
            NEXT(*cu++, medi);
            NEXT(*cu++, low);
            return { ((fst & 0b00000111) << 18) | (high << 12) | (medi << 6) | low, cu };
        }

#undef NEXT

        if constexpr(TUseException)
            throw UTFException("Decoding invalid utf-8 sequence");
        else
            return { 0, nullptr };
    }

    /**
     * @brief 将一个CodePoint转换为几个CodeUnit
     * @param cp CodePoint值
     * @param cu 用于存放输出的CodeUnit的缓冲区，至少应能容纳 MAX_CU_COUNT_IN_ONE_CP 个CodeUnit
     * @return 该CodePoint占据了几个CodeUnit的空间
     *          若没有启用异常，则在CodePoint值不合法时返回0
     * @exception UTFException 若启用了异常，则在遇到不合法的CodePoint值时抛出
     */
    static size_t Encode(CodePoint cp, CodeUnit *cu) noexcept(!TUseException)
    {
        AGZ_ASSERT(cu);

        if(cp <= 0x7f)
        {
            cu[0] = static_cast<CodeUnit>(cp);
            return 1;
        }

        if(cp <= 0x7ff)
        {
            cu[0] = static_cast<CodeUnit>(0b11000000 | (cp >> 6));
            cu[1] = static_cast<CodeUnit>(0b10000000 | (cp & 0b00111111));
            return 2;
        }

        if(cp < 0xffff)
        {
            cu[0] = static_cast<CodeUnit>(0b11100000 | (cp >> 12));
            cu[1] = static_cast<CodeUnit>(0b10000000 | ((cp >> 6) & 0b00111111));
            cu[2] = static_cast<CodeUnit>(0b10000000 | (cp & 0b00111111));
            return 3;
        }

        if(cp <= 0x10ffff)
        {
            cu[0] = static_cast<CodeUnit>(0b11110000 | (cp >> 18));
            cu[1] = static_cast<CodeUnit>(0b10000000 | ((cp >> 12) & 0b00111111));
            cu[2] = static_cast<CodeUnit>(0b10000000 | ((cp >> 6) & 0b00111111));
            cu[3] = static_cast<CodeUnit>(0b10000000 | (cp & 0b00111111));
            return 4;
        }

        if constexpr(TUseException)
            throw UTFException("Encoding invalid unicode codepoint to utf-8");
        else
            return 0;
    }
};

template<typename TCodeUnit, typename TCodePoint, bool TUseException = true>
class TUTF16
{
    static_assert(sizeof(TCodeUnit) == 2, "sizeof TCodeUnit of UTF-16 must be 1");
    static_assert(sizeof(TCodePoint) == 4, "sizeof TCodePoint of UTF-16 must be 4");

public:

    using CodePoint = TCodePoint;
    using CodeUnit  = TCodeUnit;

    /**
     * @brief 将一个CodePoint转换为CodeUnit时最多需要多少个CodeUnit的空间
     */
    static constexpr size_t MAX_CU_COUNT_IN_ONE_CP = 2;

    /**
     * @brief 从一系列CodeUnit的开头提取一个CodePoint
     * @param cu CodeUnit串的指针
     * @return 二元组(CP, pCU)，其中CP是提取出的CodePoint，pCU是剩余的CodeUnit串的首元素指针
     *         若没有启用异常，则当出现不合法CodeUnit串时返回(0, nullptr)
     * @exception TUseException 若启用了异常，则在遇到不合法CodeUnit串时抛出
     */
    static std::pair<CodePoint, const CodeUnit*> Decode(const CodeUnit *cu) noexcept(!TUseException)
    {
        AGZ_ASSERT(cu);

        CodePoint high = static_cast<CodePoint>(*cu++);
        if(high <= 0xd7ff || (0xe000 <= high && high <= 0xffff))
            return { high, cu };

        if(0xd800 <= high && high <= 0xdbff)
        {
            CodePoint low = static_cast<CodePoint>(*cu++);
            if(low <= 0xdfff)
                return { 0x10000 + (((high & 0x3ff) << 10) | (low & 0x3ff)), cu };
        }

        if constexpr(TUseException)
            throw UTFException("Decoding invalid utf-16 sequence");
        else
            return { 0, nullptr };
    }

    /**
     * @brief 将一个CodePoint转换为几个CodeUnit
     * @param cp CodePoint值
     * @param cu 用于存放输出的CodeUnit的缓冲区，至少应能容纳 MAX_CU_COUNT_IN_ONE_CP 个CodeUnit
     * @return 该CodePoint占据了几个CodeUnit的空间
     *          若没有启用异常，则在CodePoint值不合法时返回0
     * @exception UTFException 若启用了异常，则在遇到不合法的CodePoint值时抛出
     */
    static size_t Encode(CodePoint cp, CodeUnit *cu) noexcept(!TUseException)
    {
        if(cp <= 0xd7ff || (0xe000 <= cp && cp <= 0xffff))
        {
            *cu = static_cast<CodeUnit>(cp);
            return 1;
        }

        if(0x10000 <= cp && cp <= 0x10ffff)
        {
            cp -= 0x10000;
            *cu++ = static_cast<CodeUnit>(0xd800 | (cp >> 10));
            *cu = static_cast<CodeUnit>(0xdc00 | (cp & 0x3ff));
            return 2;
        }

        if constexpr(TUseException)
            throw UTFException("Encoding invalid unicode codepoint to utf-16");
        else
            return 0;
    }
};

} // namespace AGZ::Str
