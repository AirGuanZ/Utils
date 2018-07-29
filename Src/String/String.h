#pragma once

#include <atomic>
#include <stdexcept>
#include <vector>

#include "../Misc/Common.h"

#include "UTF8.h"
#include "UTF32.h"

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

    void Init(const typename CS::CodeUnit *beg, const typename CS::CodeUnit *end);

public:

    using CharSet   = CS;
    using CodeUnit  = typename CS::CodeUnit;
    using CodePoint = typename CS::CodePoint;
    using Self      = String<CS, TP>;

    String();

    // Construct from existed buffer
    // Will copy all data to owned storage
    // Faster than with-checking edition. UB when [beg, end) is invalid
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

template<typename CS, typename TP>
void String<CS, TP>::Init(const typename CS::CodeUnit *beg,
                          const typename CS::CodeUnit *end)
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

template<typename CS, typename TP>
String<CS, TP>::String()
{
    small_.len = 0;
}

template<typename CS, typename TP>
String<CS, TP>::String(CONS_FLAG_NOCHECK_t, const CodeUnit *beg,
                                            const CodeUnit *end)
{
    Init(beg, end);
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
    Init(beg, end);
}

template<typename CS, typename TP>
String<CS, TP>::String(const Self &copyFrom)
{
    if(copyFrom.IsSmallStorage())
    {
        small_.len = static_cast<uint8_t>(copyFrom.GetSmallLen());
        StringAux::CopyConstruct(&small_.buf[0], &copyFrom.small_.buf[0],
                                                 copyFrom.small_.len);
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
        CodeUnit sgl[CS::MaxCUInCP];
        while(beg < end)
        {
            typename OCS::CodePoint ocp;
            size_t skip = OCS::CU2CP(beg, &ocp, end - beg);
            if(!skip)
            {
                throw EncodingException("Input [beg, end) is not a valid "
                                      + OCS::Name() + " sequence");
            }
            beg += skip;

            size_t sgls = CS::CP2CU(CS::template From<OCS>(ocp), sgl);
            AGZ_ASSERT(sgls);
            for(size_t i = 0; i < sgls; ++i)
                cus.push_back(sgl[i]);
        }

        Init(cus.data(), cus.data() + cus.size());
    }
}

template<typename CS, typename TP>
template<typename OCS>
String<CS, TP>::String(CONS_FLAG_FROM_t<OCS>, const typename OCS::CodeUnit *beg,
                                              size_t n)
    : String(FROM<OCS>, beg, beg + n)
{

}

template<typename CS, typename TP>
template<typename OCS, typename OTP>
String<CS, TP>::String(const String<OCS, OTP>& copyFrom)
    : String(FROM<OCS>, copyFrom.Data(), copyFrom.Length())
{

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
typename String<CS, TP>::CodeUnit String<CS, TP>::operator[](size_t idx) const
{
    AGZ_ASSERT(idx < GetLen());
    return Data()[idx];
}

using Str8  = String<UTF8<>>;
using Str32 = String<UTF32<>>;

AGZ_NS_END(AGZ)
