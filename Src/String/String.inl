#include <cstring>
#include <string>
#include <vector>

AGZ_NS_BEG(AGZ)

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
String<CS, TP>::String(Self &&moveFrom)
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
typename String<CS, TP>::Self &String<CS, TP>::operator=(Self &&moveFrom)
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
    : String(cppStr.c_str(), encoding)
{

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
typename String<CS, TP>::Self &String<CS, TP>::Exchange(const Self &value)
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
        return std::string(Data(), Length());
    else
    {
        String<UTF8<char>, TP> t(*this);
        return t.ToStdString();
    }
}

template<typename CS, typename TP>
typename String<CS, TP>::Self String<CS, TP>::operator+(const Self &rhs)
{
    return Self(NOCHECK, Data(), Data() + Length(),
                         rhs.Data(), rhs.Data() + Length());
}

template<typename CS, typename TP>
typename String<CS, TP>::Self String<CS, TP>::operator*(size_t n)
{
    return Self(Data(), Data() + Length(), n);
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

AGZ_NS_END(AGZ)
