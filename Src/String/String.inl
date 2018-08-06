#pragma once

#include <cctype>
#include <cstring>
#include <string>
#include <tuple>
#include <vector>

AGZ_NS_BEG(AGZ)

namespace StringAux
{
    template<typename T>
    void CopyConstruct(T *dst, const T *src, size_t n)
    {
        for(size_t i = 0; i < n; ++i)
            new(dst++) T(*src++);
    }

    template<typename CU, typename TP>
    typename RefCountedBuf<CU, TP>::Self *RefCountedBuf<CU, TP>::New(size_t n)
    {
        AGZ_ASSERT(n >= 1);
        size_t bytes = sizeof(Self) + (n-1) * sizeof(CU);
        Self *ret = reinterpret_cast<Self*>(std::malloc(bytes));
        ret->refs = 1;
        return ret;
    }
}

template<typename CS, typename TP>
CharRange<CS, TP>::CharRange(LargeBuf *buf, const CodeUnit *beg,
                                            const CodeUnit *end)
    : largeBuf_(buf), small_(false), beg_(beg), end_(end)
{
    AGZ_ASSERT(beg <= end && buf);
    buf->IncRef();
}

template<typename CS, typename TP>
CharRange<CS, TP>::CharRange(const CodeUnit *beg, const CodeUnit *end)
    : small_(true)
{
    AGZ_ASSERT(beg <= end && end - beg <= SMALL_BUF_SIZE);
    StringAux::CopyConstruct(&smallBuf_[0], beg, end - beg);
    beg_ = &smallBuf_[0];
    end_ = beg_ + (end - beg);
}

template<typename CS, typename TP>
CharRange<CS, TP>::CharRange(const Self &copyFrom)
    : small_(copyFrom.small_)
{
    if(small_)
    {
        StringAux::CopyConstruct(smallBuf_, copyFrom.smallBuf_, SMALL_BUF_SIZE);
        beg_ = &smallBuf_[0];
        end_ = beg_ + (copyFrom.end_ - copyFrom.beg_);
    }
    else
    {
        largeBuf_ = copyFrom.largeBuf_;
        largeBuf_->IncRef();
        beg_ = copyFrom.beg_;
        end_ = copyFrom.end_;
    }
}

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
const typename CS::CodeUnit *String<CS, TP>::End() const
{
    return IsSmallStorage() ? &small_.buf[0] + GetSmallLen() :
                              large_.buf->GetData() + GetLargeLen();
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
void String<CS, TP>::Init2(const typename CS::CodeUnit *beg1,
                           const typename CS::CodeUnit *end1,
                           const typename CS::CodeUnit *beg2,
                           const typename CS::CodeUnit *end2)
{
    AGZ_ASSERT(beg1 <= end1 && beg2 <= end2);
    size_t len1 = end1 - beg1, len2 = end2 - beg2, len = len1 + len2;
    if(len <= SMALL_BUF_SIZE) // Small storage
    {
        StringAux::CopyConstruct(&small_.buf[0], beg1, len1);
        StringAux::CopyConstruct(&small_.buf[0] + len1, beg2, len2);
        small_.len = static_cast<uint8_t>(len);
    }
    else // Large storage
    {
        small_.len = SMALL_BUF_SIZE + 1;
        large_.buf = LargeBuf::New(len);
        large_.beg = large_.buf->GetData();
        large_.end = large_.beg + len;
        StringAux::CopyConstruct(large_.buf->GetData(), beg1, len1);
        StringAux::CopyConstruct(large_.buf->GetData() + len1, beg2, len2);
    }
}

template<typename CS, typename TP>
String<CS, TP> &String<CS, TP>::CopyFromSelf(const Self &copyFrom)
{
    if(IsLargeStorage())
        large_.buf->DecRef();
    ConstructFromSelf(copyFrom);
    return *this;
}

template<typename CS, typename TP>
void String<CS, TP>::ConstructFromSelf(const Self &copyFrom)
{
    if(copyFrom.IsSmallStorage())
    {
        static_assert(std::is_trivially_copyable_v<CodeUnit>);
        std::memcpy(&small_, &copyFrom.small_, sizeof(small_));
    }
    else
    {
        small_.len = copyFrom.small_.len;
        large_ = copyFrom.large_;
        large_.buf->IncRef();
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
{
    if(!CS::Check(beg, end - beg))
    {
        throw EncodingException("Input [beg, end) is not a valid "
                              + CS::Name() + " sequence");
    }
    Init(beg, end);
}

template<typename CS, typename TP>
String<CS, TP>::String(CONS_FLAG_NOCHECK_t,
                       const CodeUnit* beg1, const CodeUnit* end1,
                       const CodeUnit* beg2, const CodeUnit* end2)
{
    Init2(beg1, end1, beg2, end2);
}

template<typename CS, typename TP>
String<CS, TP>::String(const CodeUnit* beg1, const CodeUnit* end1,
                       const CodeUnit* beg2, const CodeUnit* end2)
{
    if(!CS::Check(beg1, end1 - beg1) || !CS::Check(beg2, end2 - beg2))
    {
        throw EncodingException("Input [beg, end) is not a valid "
                              + CS::Name() + " sequence");
    }
    Init2(beg1, end1, beg2, end2);
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
String<CS, TP>::String(const Self &copyFrom)
{
    ConstructFromSelf(copyFrom);
}

template<typename CS, typename TP>
String<CS, TP>::String(Self &&moveFrom) noexcept
{
    if(moveFrom.IsSmallStorage())
    {
        static_assert(std::is_trivially_copyable_v<CodeUnit>);
        std::memcpy(&small_, &moveFrom.small_, sizeof(small_));
    }
    else
    {
        large_ = moveFrom.large_;
        small_.len = SMALL_BUF_SIZE + 1;
        moveFrom.small_.len = 0;
    }
}

template<typename CS, typename TP>
template<typename OCS, typename OTP>
String<CS, TP> &String<CS, TP>::operator=(const String<OCS, OTP> &copyFrom)
{
    if constexpr(std::is_same_v<CS, OCS>)
        return CopyFromSelf(copyFrom);
    else
        return *this = Self(FROM<OCS>, copyFrom.Data(),
                            copyFrom.Data() + copyFrom.Length());
}

template<typename CS, typename TP>
typename String<CS, TP>::Self &
String<CS, TP>::operator=(Self &&moveFrom) noexcept
{
    if(IsLargeStorage())
        large_.buf->DecRef();
    if(moveFrom.IsSmallStorage())
    {
        static_assert(std::is_trivially_copyable_v<CodeUnit>);
        std::memcpy(&small_, &moveFrom.small_, sizeof(small_));
    }
    else
    {
        small_.len = moveFrom.small_.len;
        large_ = moveFrom.large_;
        moveFrom.small_.len = 0;
    }
    return *this;
}

template<typename CS, typename TP>
String<CS, TP> &String<CS, TP>::operator=(const Self &copyFrom)
{
    CopyFromSelf(copyFrom);
    return *this;
}

template<typename CS, typename TP>
String<CS, TP>::String(const CodeUnit *beg, const CodeUnit *end, size_t repeat)
{
    AGZ_ASSERT(beg <= end);
    size_t ulen = end - beg, tlen = ulen * repeat;
    if(!ulen)
    {
        new(this) Self();
        return;
    }

    if(tlen <= SMALL_BUF_SIZE)
    {
        CodeUnit *buf = &small_.buf[0];
        for(size_t i = 0; i < repeat; ++i, buf += ulen)
            StringAux::CopyConstruct(buf, beg, ulen);
        small_.len = static_cast<uint8_t>(tlen);
    }
    else
    {
        small_.len = SMALL_BUF_SIZE + 1;
        large_.buf = LargeBuf::New(tlen);
        large_.beg = large_.buf->GetData();
        large_.end = large_.beg + tlen;

        CodeUnit *buf = large_.buf->GetData();
        for(size_t i = 0; i < repeat; ++i, buf += ulen)
            StringAux::CopyConstruct(buf, beg, ulen);
    }
}

template<typename CS, typename TP>
String<CS, TP>::String(const char *cStr)
    : String(cStr, CharEncoding::UTF8)
{

}

template<typename CS, typename TP>
String<CS, TP>::String(const std::string &cppStr)
    : String(cppStr, CharEncoding::UTF8)
{

}

template<typename CS, typename TP>
String<CS, TP>::String(const char *cStr, CharEncoding encoding)
{
    switch(encoding)
    {
    case CharEncoding::UTF8:
        new(this) Self(FROM<UTF8<char>>, cStr, std::strlen(cStr));
        break;
    default:
        throw EncodingException("Unknown encoding: "
                + std::to_string(
                    static_cast<std::underlying_type_t<CharEncoding>>
                    (encoding)));
    }
}

template<typename CS, typename TP>
String<CS, TP>::String(const std::string &cppStr, CharEncoding encoding)
{
    switch(encoding)
    {
    case CharEncoding::UTF8:
        new(this) Self(FROM<UTF8<char>>, cppStr.c_str(), cppStr.length());
        break;
    default:
        throw EncodingException("Unknown encoding: "
                + std::to_string(
                    static_cast<std::underlying_type_t<CharEncoding>>
                    (encoding)));
    }
}

template<typename CS, typename TP>
String<CS, TP>::String(const Self &copyFrom, size_t beg, size_t end)
{
    AGZ_ASSERT(beg <= end);
    if(beg == end)
    {
        small_.len = 0;
    }
    else if(copyFrom.IsSmallStorage())
    {
        auto d = copyFrom.Data()
        Init(d + beg, d + end);
    }
    else
    {
        small_.len = copyFrom.small_.len;
        large_.buf = copyFrom.large_.buf;
        large_.buf->IncRef();
        large_.beg = large_.buf->GetData() + beg;
        large_.beg = large_.buf->GetData() + end;
    }
}

template<typename CS, typename TP>
String<CS, TP>::~String()
{
    if(IsLargeStorage())
        large_.buf->DecRef();
}

template<typename CS, typename TP>
void String<CS, TP>::Swap(Self &other)
{
    // IMPROVE
    Self t = *this;
    *this = other;
    other = t;
}

template<typename CS, typename TP>
typename String<CS, TP>::Self String<CS, TP>::Exchange(const Self &value)
{
    // IMPROVE
    Self ret = *this;
    *this = value;
    return ret;
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

template<typename CS, typename TP>
bool String<CS, TP>::IsEmpty() const
{
    return Length() == 0;
}

template<typename CS, typename TP>
std::pair<const typename String<CS, TP>::CodeUnit*, size_t>
String<CS, TP>::DataAndLength() const
{
    if(IsSmallStorage())
        return { &small_.buf[0], static_cast<size_t>(small_.len) };
    return { large_.beg, large_.end - large_.beg };
}

template<typename CS, typename TP>
std::pair<const typename String<CS, TP>::CodeUnit*,
          const typename String<CS, TP>::CodeUnit*>
String<CS, TP>::BeginAndEnd() const
{
    if(IsSmallStorage())
        return { &small_.buf[0], &small_.buf[0] + small_.len };
    return { large_.beg, large_.end };
}

template <typename CS, typename TP>
typename String<CS, TP>::CodeUnit String<CS, TP>::operator[](size_t idx) const
{
    AGZ_ASSERT(idx < GetLen());
    return Data()[idx];
}

template<typename CS, typename TP>
std::string String<CS, TP>::ToStdString() const
{
    if constexpr (std::is_same_v<CodeUnit, char>)
        return std::make_from_tuple<std::string>(DataAndLength());
    else
    {
        String<UTF8<char>, TP> t(*this);
        return t.ToStdString();
    }
}

template<typename CS, typename TP>
typename String<CS, TP>::Self String<CS, TP>::operator+(const Self &rhs)
{
    auto args = std::tuple_cat(std::make_tuple(NOCHECK), BeginAndEnd(), rhs.BeginAndEnd());
    return std::make_from_tuple<Self>(args);
}

template<typename CS, typename TP>
typename String<CS, TP>::Self String<CS, TP>::operator*(size_t n)
{
    return Self(Data(), Data() + Length(), n);
}

template<typename CS, typename TP>
CharRange<CS, TP> String<CS, TP>::Chars() const
{
    if(IsSmallStorage())
        return CharRange<CS, TP>(Data(), End());
    return CharRange<CS, TP>(large_.buf, Data(), End());
}

template<typename CS, typename TP>
typename String<CS, TP>::Iterator String<CS, TP>::begin() const
{
    return Data();
}

template<typename CS, typename TP>
typename String<CS, TP>::Iterator String<CS, TP>::end() const
{
    return End();
}

template<typename CS, typename TP>
bool String<CS, TP>::StartsWith(const Self &prefix) const
{
    return StrAlgo::StartsWith(begin(), end(),
                               std::begin(prefix), std::end(prefix));
}

template<typename CS, typename TP>
bool String<CS, TP>::EndsWith(const Self &suffix) const
{
    return StrAlgo::EndsWith(begin(), end(),
                                std::begin(suffix), std::end(suffix));
}

template<typename CS, typename TP>
size_t String<CS, TP>::Find(const Self &dst, size_t beg) const
{
    if(beg >= Length())
        return NPOS;
    return StrAlgo::Find(begin() + beg, end(),
                         std::begin(dst), std::end(dst));
}

template<typename CS, typename TP>
size_t String<CS, TP>::RFind(const Self &dst, size_t rbeg) const
{
    if(rbeg >= Length())
        return NPOS;
    return StrAlgo::RFind(begin(), end() - rbeg,
                          std::begin(dst), std::end(dst));
}

template<typename CS, typename TP>
String<CS, TP> String<CS, TP>::Slice(size_t beg, size_t end) const
{
    AGZ_ASSERT(beg <= end && end <= Length());
    return Self(*this, beg, end);
}

template<typename CS, typename TP>
String<CS, TP> String<CS, TP>::Slice(size_t beg) const
{
    return Slice(beg, Length());
}

template<typename CS, typename TP>
String<CS, TP> String<CS, TP>::LeftStrip() const
{
    size_t idx = 0;
    auto [data, len] = DataAndLength();
    while(idx < len && std::isspace(data[idx]))
        ++idx;
    return Slice(idx);
}

template<typename CS, typename TP>
String<CS, TP> String<CS, TP>::RightStrip() const
{
    auto [data, end] = DataAndLength();
    while(end > 0 && std::isspace(data[idx]))
        --end;
    return Slice(0, end);
}

template<typename CS, typename TP>
String<CS, TP> String<CS, TP>::Strip() const
{
    return LeftStrip().RightStrip();
}

template<typename CS, typename TP>
std::vector<String<CS, TP>> String<CS, TP>::Split(
                    const std::vector<Self> &spliters)
{
    std::vector<Self> ret; Self s = *this;

    while(!s.IsEmpty())
    {
        size_t splitBeg = NPOS, splitLen = 0;
        for(auto &spliter : rhs.spliters)
        {
            AGZ_ASSERT(!spliter.IsEmpty());
            size_t beg = s.Find(spliter);
            if(beg < splitBeg)
            {
                splitBeg = beg;
                splitLen = spliter.Length();
            }
        }

        if(splitBeg == NPOS)
        {
            ret.push_back(std::move(s));
            return std::move(ret);
        }

        if(splitBeg != 0)
            ret.push_back(s.Slice(0, splitBeg));

        s = s.Slice(splitBeg + splitLen);
    }

    return std::move(ret);
}

template<typename CS, typename TP>
std::vector<String<CS, TP>> String<CS, TP>::Lines()
{
    return Split({ u8"\n", u8"\r\n" });
}

template<typename CS, typename TP>
template<typename R>
String<CS, TP> String<CS, TP>::Join(const R &strs) const
{
    return strs | Join(*this);
}

template<typename CS, typename TP>
bool String<CS, TP>::operator==(const Self &rhs) const
{
    auto [b1, e1] = BeginAndEnd();
    auto [b2, e2] = rhs.BeginAndEnd();
    return StrAlgo::Compare(b1, e1, b2, e2)
        == StrAlgo::CompareResult::Equal;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator!=(const Self &rhs) const
{
    auto [b1, e1] = BeginAndEnd();
    auto [b2, e2] = rhs.BeginAndEnd();
    return StrAlgo::Compare(b1, e1, b2, e2)
        != StrAlgo::CompareResult::Equal;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator<(const Self &rhs) const
{
    auto [b1, e1] = BeginAndEnd();
    auto [b2, e2] = rhs.BeginAndEnd();
    return StrAlgo::Compare(b1, e1, b2, e2)
        == StrAlgo::CompareResult::Less;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator<=(const Self &rhs) const
{
    auto [b1, e1] = BeginAndEnd();
    auto [b2, e2] = rhs.BeginAndEnd();
    return StrAlgo::Compare(b1, e1, b2, e2)
        != StrAlgo::CompareResult::Greater;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator>=(const Self &rhs) const
{
    auto [b1, e1] = BeginAndEnd();
    auto [b2, e2] = rhs.BeginAndEnd();
    return StrAlgo::Compare(b1, e1, b2, e2)
        != StrAlgo::CompareResult::Less;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator>(const Self &rhs) const
{
    auto [b1, e1] = BeginAndEnd();
    auto [b2, e2] = rhs.BeginAndEnd();
    return StrAlgo::Compare(b1, e1, b2, e2)
        == StrAlgo::CompareResult::Greater;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator==(const std::string &rhs) const
{
    return ToStdString() == rhs;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator!=(const std::string &rhs) const
{
    return ToStdString() != rhs;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator<(const std::string &rhs) const
{
    return ToStdString() < rhs;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator<=(const std::string &rhs) const
{
    return ToStdString() <= rhs;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator>=(const std::string &rhs) const
{
    return ToStdString() >= rhs;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator>(const std::string &rhs) const
{
    return ToStdString() > rhs;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator==(const char *rhs) const
{
    return ToStdString() == rhs;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator!=(const char *rhs) const
{
    return ToStdString() != rhs;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator<(const char *rhs) const
{
    return ToStdString() < rhs;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator<=(const char *rhs) const
{
    return ToStdString() <= rhs;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator>=(const char *rhs) const
{
    return ToStdString() >= rhs;
}

template<typename CS, typename TP>
bool String<CS, TP>::operator>(const char *rhs) const
{
    return ToStdString() > rhs;
}

template<typename CS, typename TP, typename R>
String<CS, TP> &operator+=(String<CS, TP> &lhs, R &&rhs)
{
    return lhs = lhs + std::forward<R>(rhs);
}

template<typename CS, typename TP>
bool operator==(const std::string &lhs, const String<CS, TP> &rhs)
{
    return lhs == rhs.ToStdString();
}

template<typename CS, typename TP>
bool operator!=(const std::string &lhs, const String<CS, TP> &rhs)
{
    return lhs != rhs.ToStdString();
}

template<typename CS, typename TP>
bool operator<(const std::string &lhs, const String<CS, TP> &rhs)
{
    return lhs < rhs.ToStdString();
}

template<typename CS, typename TP>
bool operator<=(const std::string &lhs, const String<CS, TP> &rhs)
{
    return lhs <= rhs.ToStdString();
}

template<typename CS, typename TP>
bool operator>=(const std::string &lhs, const String<CS, TP> &rhs)
{
    return lhs >= rhs.ToStdString();
}

template<typename CS, typename TP>
bool operator>(const std::string &lhs, const String<CS, TP> &rhs)
{
    return lhs > rhs.ToStdString();
}

template<typename CS, typename TP>
bool operator==(const char *lhs, const String<CS, TP> &rhs)
{
    return lhs == rhs.ToStdString();
}

template<typename CS, typename TP>
bool operator!=(const char *lhs, const String<CS, TP> &rhs)
{
    return lhs != rhs.ToStdString();
}

template<typename CS, typename TP>
bool operator<(const char *lhs, const String<CS, TP> &rhs)
{
    return lhs < rhs.ToStdString();
}

template<typename CS, typename TP>
bool operator<=(const char *lhs, const String<CS, TP> &rhs)
{
    return lhs <= rhs.ToStdString();
}

template<typename CS, typename TP>
bool operator>=(const char *lhs, const String<CS, TP> &rhs)
{
    return lhs >= rhs.ToStdString();
}

template<typename CS, typename TP>
bool operator>(const char *lhs, const String<CS, TP> &rhs)
{
    return lhs > rhs.ToStdString();
}

template<typename CS, typename TP>
String<CS, TP> operator*(size_t n, const String<CS, TP> &s)
{
    return s * n;
}

template<typename CS, typename TP>
std::ostream &operator<<(std::ostream &out, const String<CS, TP> &s)
{
    return out << s.ToStdString();
}

inline StringJoinRHS Join(const Str8 &mid, const Str8 &empty)
{
    return StringJoinRHS{ mid, empty };
}

template<typename R>
auto operator|(const R &strs, const StringJoinRHS &rhs)
{
    // IMPROVE

    using RT = typename R::value_type;
    if(strs.empty())
        return RT(rhs.empty);
    RT ret = strs[0];
    auto cur = std::begin(strs), end = std::end(strs);
    while(++cur != end)
        ret += rhs.mid + *cur;
    return std::move(ret);
}

inline StringSplitRHS Split(const std::vector<Str8> &spliters =
                            { u8" ", u8"\t", u8"\n", u8"\r\n" })
{
    return StringSplitRHS { spliters };
}

AGZ_NS_END(AGZ)
