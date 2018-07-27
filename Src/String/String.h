#pragma once

#include <atomic>
#include <cstring>
#include <type_traits>

#include "../Misc/Common.h"
#include "../Misc/Option.h"

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

template<typename CU, typename CP, typename TP> class TString;

namespace StringAux
{
    template<typename T, std::enable_if_t<
        std::is_trivially_copiable_v<T>, int> = 0>
    void Construct(T *dst, const T *src, size_t n)
    {
        std::memcpy(dst, src, n * sizeof(T));
    }

    template<typename T, std::enable_if_t<
        !std::is_trivially_copiable_v<T>, int> = 0>
    void Construct(T *dst, const T *src, size_t n)
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

    // CodeUnitSequence中的坐标
    // 只能由TString创建，保证一定指向一个合法CodePoint的开头
    // 用户可以通过这种location来标定String中的位置，譬如创建子串等
    template<typename CU, typename CP, typename TP>
    class TStrLoc
    {
        const CU *pos;

        TStrLoc(const CU *pos) : pos(pos) { }

    public:

        friend class AGZ::TString<CU, CP, TP>;
    };
}

// CU: Code Unit
// CP: Code Point
// TP: Thread Policy
template<typename CU, typename CP,
         typename TP = StringAux::SingleThreaded>
class TString
{
    static constexpr INT_BUF_SIZE = 31;

    using LargeBuf = RefCountedBuf<CU, TP>;

    union
    {
        struct
        {
            CU buf[INT_BUF_SIZE];
            CU len;
        } small_;

        struct
        {
            LargeBuf *buf;
            const CU *beg;
            const CU *end;
        } large_;
    };

    bool IsSmallStorage() const;
    bool IsLargeStorage() const;

    size_t GetSmallLen() const;
    size_t GetLargeLen() const;
    size_t GetLen() const;

public:

    using CodeUnit  = CU;
    using CodePoint = CP;
    using Self      = TString<CU, CT>;

    // Construct from existed buffer. Will copy all data to owned storage.
    TString(const CU *beg, const CU *end);

    TString(const Self &copyFrom);

    ~TString();

    const CU *RawCodeUnits() const;
    size_t CodeUnitCount() const;
};

template<typename CU, typename CP, typename TP>
inline bool TString<CU, CP, TP>::IsSmallStorage() const
{
    return small_.len <= INT_BUF_SIZE;
}

template<typename CU, typename CP, typename TP>
inline bool TString<CU, CP, TP>::IsLargeStorage() const
{
    return small_.len > INT_BUF_SIZE;
}

template<typename CU, typename CP, typename TP>
inline size_t TString<CU, CP, TP>::GetSmallLen() const
{
    AGZ_ASSERT(IsSmallStorage());
    return small_.len;
}

template<typename CU, typename CP, typename TP>
inline size_t TString<CU, CP, TP>::GetLargeLen() const
{
    AGZ_ASSERT(IsLargeStorage() && large_.beg < large_.end);
    return large_.end - large_.beg;
}

template<typename CU, typename CP, typename TP>
inline size_t TString<CU, CP, TP>::GetLen() const
{
    return IsSmallStorage() ? GetSmallLen() : GetLargeLen();
}

template<typename CU, typename CP, typename TP>
inline TString<CU, CP, TP>::TString(const CU *beg, const CU *end)
{
    AGZ_ASSERT(beg <= end);
    size_t len = end - beg;
    if(len <= INT_BUF_SIZE) // Small storage
    {
        StringAux::Construct<CU>(&small_[0], beg, len);
        small_.len = len;
    }
    else // Large storage
    {
        small_.len = INT_BUF_SIZE + 1;
        large_.buf = LargeBuf::New(len);
        large_.beg = large_.buf.GetData();
        large_.end = large_.beg + len;
        StringAux::Cnostruct<CU>(large_.buf.GetData(), beg, len);
    }
}

template<typename CU, typename CP, typename TP>
inline TString<CU, CP, TP>::TString(const Self &copyFrom)
{
    if(copyFrom.IsSmallStorage())
    {
        small_.len = copyFrom.GetSmallLen();
        StringAux::Construct<CU>(&small_.buf[0], &copyFrom.small_.buf[0],
                                 small_.len);
    }
    else
    {
        small_.len = INT_BUF_SIZE + 1;
        large_.buf = copyFrom.large_.buf;
        large_.beg = copyFrom.large_.beg;
        large_.end = copyFrom.large_.end;
        large_.buf->IncRef();
    }
}

template<typename CU, typename CP, typename TP>
inline TString<CU, CP, TP>::~TString()
{
    if(IsLargeStorage())
        large_.buf->DecRef();
}

template<typename CU, typename CP, typename TP>
inline const CU *TString<CU, CP, TP>::RawCodeUnits() const
{
    return IsSmallStorage() ? &small_.buf[0] : large_.beg;
}

template<typename CU, typename CP, typename TP>
inline size_t TString<CU, CP, TP>::CodeUnitCount() const
{
    return GetLen();
}

AGZ_NS_END(AGZ)
