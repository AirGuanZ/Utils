#pragma once

#include <atomic>
#include <stdexcept>
#include <vector>

#include "../Misc/Common.h"
#include "UTF8.h"

/*
    撇开std::string的蛋疼不提，即使是在使用Rust的String的时候，我也无法感到称心如意。
    在网络上可以看到很多关于这种不快来源的言论，譬如可变/不可变、动态类型/静态类型（放屁）
    云云。

    一种我比较认同的观点是：String不应该是Char的序列。的确，从存储机制上，
    认为String是Char序列是再自然不过的想法，但这样一来String就沦落为vector<Char>了
    （C++正是这么干的）。

    这使得我们可以自由地操控String中的每个CodeUnit，却在涉及到CodePoint及更高的抽象层次
    的操作上心惊胆战，毕竟vector<Char>根本就没有提供“字符串”这一概念应有的抽象。
    因此，我打算试着彻底摈弃把String看作Char序列的做法，从AGZ::String类的接口中，将完全
    无法以Char的方式访问其内容，即使是for each这样的迭代操作，AGZ::String也只会提供
    immutable的、仅包含一个UTF8 CodePoint的字符串。

    存储方面，对小型字符串，使用一块固定buffer直接拷贝；对稍微大些的字符串，用引用计数
    进行共享。

    small_.len记录small string的长度。若超过INT_BUF_SIZE，则为large。
    换言之，这里的存储并不是0-ended的。

    另外，因为是Immutable的，所以线程安全不成问题。唯一需要注意的是引用计数的跨线程操作，
    这一点由本实现自行保证。
*/

AGZ_NS_BEG(AGZ)

template<typename CS, typename TP> class String;

namespace StringAux
{
    template<typename T>
    void CopyConstruct(T *dst, const T *src, size_t n)
    {
        for(size_t i = 0; i < n; ++i)
            new(dst++) T(*src++);
    }

    struct MultiThreaded
    {
        using RefCounter = std::atomic<size_t>;
        static constexpr bool IsThreadSafe = true;
    };

    struct SingleThreaded
    {
        using RefCounter = size_t;
        static constexpr bool IsThreadSafe = false;
    };

    template<typename CU, typename TP>
    class RefCountedBuf
    {
        mutable typename TP::RefCounter refs;
        CU data[1];

    public:

        using RefCounter = typename TP::RefCounter;
        using Self = RefCountedBuf<CU, TP>;

        static constexpr bool IsThreadSafe = TP::IsThreadSafe;

        RefCountedBuf()               = delete;
        RefCountedBuf(const Self &)   = delete;
        ~RefCountedBuf()              = delete;
        
        Self &operator=(const Self &) = delete;

        void IncRef() const { ++refs; }
        void DecRef() { if(!--refs) std::free(this); }

        CU *GetData() { return &data[0]; }

        static Self *New(size_t n)
        {
            AGZ_ASSERT(n >= 1);
            size_t bytes = sizeof(Self) + (n-1) * sizeof(CU);
            Self *ret = reinterpret_cast<Self*>(std::malloc(bytes));
            ret->refs = 1;
            return ret;
        }
    };
}

class EncodingException : public std::invalid_argument
{
public:

    EncodingException(const std::string &err) : invalid_argument(err) { }
};

// CU: Code Unit
// CP: Code Point
// TP: Thread Policy
template<typename CS = UTF8<char>, typename TP = StringAux::MultiThreaded>
class String
{
    static constexpr size_t SMALL_BUF_SIZE = 31;

    using LargeBuf = StringAux::RefCountedBuf<typename CS::CodeUnit, TP>;

    union
    {
        struct
        {
            typename CS::CodeUnit buf[SMALL_BUF_SIZE];
            std::uint8_t len;
        } small_;

        struct
        {
            LargeBuf *buf;
            const typename CS::CodeUnit *beg;
            const typename CS::CodeUnit *end;
        } large_;
    };

    bool IsSmallStorage() const;
    bool IsLargeStorage() const;

    size_t GetSmallLen() const;
    size_t GetLargeLen() const;
    size_t GetLen() const;

    typename CS::CodeUnit *GetRawBuf();

public:

    using CharSet   = CS;
    using CodeUnit  = typename CS::CodeUnit;
    using CodePoint = typename CS::CodePoint;
    using Self      = String<CS, TP>;

    String();

    // Construct from existed buffer
    // Will copy all data to owned storage
    // Doesn't check the correctness of given code units. UB when [beg, end) is invalid
    String(CONS_FLAG_NOCHECK_t, const CodeUnit *beg, const CodeUnit *end);

    // Construct from existed buffer
    // Will copy all data to owned storage
    String(const CodeUnit *beg, const CodeUnit *end);

    // Construct from existed buffer
    // Will copy all data to owned storage
    String(const CodeUnit *beg, size_t n) : String(beg, beg + n) { }

    template<typename OCS>
    String(CONS_FLAG_FROM_t<OCS>, const typename OCS::CodeUnit *beg,
                                  const typename OCS::CodeUnit *end);


    template<typename OCS>
    String(CONS_FLAG_FROM_t<OCS>, const typename OCS::CodeUnit *beg,
                                  size_t n);

    template<typename OCS, typename OTP>
    String(const String<OCS, OTP> &copyFrom);

    String(const Self &copyFrom);

    ~String();

    const CodeUnit *Data() const;
    size_t Length() const;

    CodeUnit &operator[](size_t idx);
    CodeUnit operator[](size_t idx) const;
};

template<typename CS, typename TP>
bool String<CS, TP>::IsSmallStorage() const
{
    return small_.len <= SMALL_BUF_SIZE;
}

template<typename CS, typename TP>
bool String<CS, TP>::IsLargeStorage() const
{
    return small_.len > SMALL_BUF_SIZE;
}

template<typename CS, typename TP>
size_t String<CS, TP>::GetSmallLen() const
{
    AGZ_ASSERT(IsSmallStorage());
    return small_.len;
}

template<typename CS, typename TP>
size_t String<CS, TP>::GetLargeLen() const
{
    AGZ_ASSERT(IsLargeStorage() && large_.beg < large_.end);
    return large_.end - large_.beg;
}

template<typename CS, typename TP>
size_t String<CS, TP>::GetLen() const
{
    return IsSmallStorage() ? GetSmallLen() : GetLargeLen();
}

template <typename CS, typename TP>
typename CS::CodeUnit* String<CS, TP>::GetRawBuf()
{
    return IsSmallStorage() ? &small_.buf[0] : large_.beg;
}

template<typename CS, typename TP>
String<CS, TP>::String()
{
    small_.len = 0;
}

template<typename CS, typename TP>
String<CS, TP>::String(CONS_FLAG_NOCHECK_t, const CodeUnit *beg, const CodeUnit *end)
{
    AGZ_ASSERT(beg <= end);
    size_t len = end - beg;
    if(len <= SMALL_BUF_SIZE) // Small storage
    {
        StringAux::CopyConstruct(&small_.buf[0], beg, len);
        small_.len = static_cast<uint8_t>(len);
    }
    else // Large storage
    {
        small_.len = SMALL_BUF_SIZE + 1;
        large_.buf = LargeBuf::New(len);
        large_.beg = large_.buf->GetData();
        large_.end = large_.beg + len;
        StringAux::CopyConstruct(large_.buf->GetData(), beg, len);
    }
}

template <typename CS, typename TP>
String<CS, TP>::String(const CodeUnit* beg, const CodeUnit* end)
    : String(NOCHECK, beg, end)
{
    if(!CS::Check(beg, end - beg))
    {
        throw EncodingException("Input [beg, end) is not a valid "
                              + CS::Name() + " sequence");
    }
}

template<typename CS, typename TP>
String<CS, TP>::String(const Self &copyFrom)
{
    if(copyFrom.IsSmallStorage())
    {
        small_.len = static_cast<uint8_t>(copyFrom.GetSmallLen());
        StringAux::CopyConstruct(&small_.buf[0], &copyFrom.small_.buf[0], copyFrom.small_.len);
    }
    else
    {
        small_.len = SMALL_BUF_SIZE + 1;
        large_.buf = copyFrom.large_.buf;
        large_.beg = copyFrom.large_.beg;
        large_.end = copyFrom.large_.end;
        large_.buf->IncRef();
    }
}

template<typename CS, typename TP>
template<typename OCS>
String<CS, TP>::String(CONS_FLAG_FROM_t<OCS>, const typename OCS::CodeUnit *beg,
                                              const typename OCS::CodeUnit *end)
{
    if constexpr (std::is_same_v<CS, OCS>)
        new(this) Self(beg, end);
    else
    {
        std::vector<CodeUnit> cus;

    }
}

template<typename CS, typename TP>
String<CS, TP>::~String()
{
    if(IsLargeStorage())
        large_.buf->DecRef();
}

template<typename CS, typename TP>
const typename String<CS, TP>::CodeUnit *String<CS, TP>::Data() const
{
    return IsSmallStorage() ? &small_.buf[0] : large_.beg;
}

template<typename CS, typename TP>
size_t String<CS, TP>::Length() const
{
    return GetLen();
}

template <typename CS, typename TP>
typename String<CS, TP>::CodeUnit& String<CS, TP>::operator[](size_t idx)
{
    AGZ_ASSERT(idx < GetLen());
    return GetRawBuf()[idx];
}

template <typename CS, typename TP>
typename String<CS, TP>::CodeUnit String<CS, TP>::operator[](size_t idx) const
{
    AGZ_ASSERT(idx < GetLen());
    return Data()[idx];
}

using Str = String<UTF8<char>>;

AGZ_NS_END(AGZ)
