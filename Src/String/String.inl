#pragma once

#include <algorithm>
#include <cstring>
#include <type_traits>

#include "../Alloc/Malloc.h"
#include "StrAlgo.inl"

AGZ_NS_BEG(AGZ::StrImpl)

template<typename E>
void Copy(const E *src, size_t cnt, E *dst)
{
    static_assert(std::is_trivially_copyable_v<E>);
    std::memcpy(dst, src, sizeof(E) * cnt);
}

template<typename E>
RefCountedBuf<E> *RefCountedBuf<E>::New(size_t n)
{
    size_t allocSize = (sizeof(Self) - sizeof(E)) + n * sizeof(E);
    Self *ret = alloc_throw<Self>(std::malloc, allocSize);
    ret->refs_ = 1;
    return ret;
}

template<typename E>
void RefCountedBuf<E>::IncRef()
{
    ++refs_;
}

template<typename E>
void RefCountedBuf<E>::DecRef()
{
    if(!--refs_)
        std::free(this);
}

template<typename E>
E *RefCountedBuf<E>::GetData()
{
    return &data_[0];
}

template<typename E>
const E *RefCountedBuf<E>::GetData() const
{
    return &data_[0];
}

template<typename CU>
void Storage<CU>::AllocSmall(size_t len)
{
    AGZ_ASSERT(len <= SMALL_BUF_SIZE);
    small_.len = static_cast<std::uint8_t>(len);
}

template<typename CU>
void Storage<CU>::AllocLarge(size_t len)
{
    small_.len = SMALL_BUF_SIZE + 1;
    large_.buf = LargeBuf::New(len);
    large_.beg = large_.buf->GetData();
    large_.end = large_.beg + len;
}

template<typename CU>
CU *Storage<CU>::GetSmallMutableData()
{
    AGZ_ASSERT(IsSmallStorage());
    return &small_.buf[0];
}

template<typename CU>
CU *Storage<CU>::GetLargeMutableData()
{
    AGZ_ASSERT(IsLargeStorage());
    auto d = large_.buf->GetData();
    return d + (large_.beg - d);
}

template<typename CU>
CU *Storage<CU>::GetMutableData()
{
    return IsSmallStorage() ? GetSmallMutableData() :
                              GetLargeMutableData();
}

template<typename CU>
Storage<CU>::Storage(size_t len)
{
    if(len <= SMALL_BUF_SIZE)
        AllocSmall(len);
    else
        AllocLarge(len);
}

template<typename CU>
Storage<CU>::Storage(const CU *data, size_t len)
{
    if(len <= SMALL_BUF_SIZE)
    {
        AllocSmall(len);
        Copy(data, len, GetSmallMutableData());
    }
    else
    {
        AllocLarge(len);
        Copy(data, len, GetLargeMutableData());
    }
}

template<typename CU>
Storage<CU>::Storage(const CU *beg, const CU *end)
    : Storage(beg, static_cast<size_t>(end - beg))
{

}

template<typename CU>
Storage<CU>::Storage(const Self &copyFrom)
    : Storage(copyFrom, 0, copyFrom.GetLength())
{

}

template<typename CU>
Storage<CU>::Storage(const Self &copyFrom, size_t beg, size_t end)
{
    AGZ_ASSERT(beg <= end);
    size_t len = end - beg;
    if(len <= SMALL_BUF_SIZE)
    {
        AllocSmall(len);
        Copy(&copyFrom.Begin()[beg], len, GetSmallMutableData());
    }
    else
    {
        small_.len = copyFrom.small_.len;
        large_.buf = copyFrom.large_.buf;
        large_.beg = &copyFrom.Begin()[beg];
        large_.end = large_.beg + len;
        large_.buf->IncRef();
    }
}

template<typename CU>
Storage<CU>::Storage(Self &&moveFrom) noexcept
{
    if(moveFrom.IsLargeStorage())
    {
        large_ = moveFrom.large_;
        small_.len = moveFrom.small_.len;
        moveFrom.small_.len = 0;
    }
    else
    {
        auto [data, len] = moveFrom.BeginAndLength();
        AllocSmall(len);
        Copy(data, len, GetSmallMutableData());
    }
}

template<typename CU>
Storage<CU>::~Storage()
{
    if(IsLargeStorage())
        large_.buf->DecRef();
}

template<typename CU>
Storage<CU> &Storage<CU>::operator=(const Self &copyFrom)
{
    if(IsLargeStorage())
        large_.buf->DecRef();
    new(this) Self(copyFrom);
    return *this;
}

template<typename CU>
Storage<CU> &Storage<CU>::operator=(Self &&moveFrom) noexcept
{
    if(IsLargeStorage())
        large_.buf->DecRef();
    new(this) Self(std::move(moveFrom));
    return *this;
}

template<typename CU>
bool Storage<CU>::IsSmallStorage() const
{
    return small_.len <= SMALL_BUF_SIZE;
}

template<typename CU>
bool Storage<CU>::IsLargeStorage() const
{
    return !IsSmallStorage();
}

template<typename CU>
size_t Storage<CU>::GetSmallLength() const
{
    AGZ_ASSERT(IsSmallStorage());
    return small_.len;
}

template<typename CU>
size_t Storage<CU>::GetLargeLength() const
{
    AGZ_ASSERT(IsLargeStorage() && large_.beg <= large_.end);
    return large_.end - large_.beg;
}

template<typename CU>
size_t Storage<CU>::GetLength() const
{
    return IsSmallStorage() ? GetSmallLength() : GetLargeLength();
}

template<typename CU>
const CU *Storage<CU>::Begin() const
{
    return IsSmallStorage() ? &small_.buf[0] : large_.beg;
}

template<typename CU>
const CU *Storage<CU>::End() const
{
    return IsSmallStorage() ? (&small_.buf[small_.len]) : large_.end;
}

template<typename CU>
std::pair<const CU*, size_t> Storage<CU>::BeginAndLength() const
{
    return IsSmallStorage() ?
        std::pair<const CU*, size_t>
            { &small_.buf[0], small_.len } :
        std::pair<const CU*, size_t>
            { large_.beg, large_.end - large_.beg };
}

template<typename CU>
std::pair<const CU*, const CU*> Storage<CU>::BeginAndEnd() const
{
    return IsSmallStorage() ?
        std::pair<const CU*, const CU*>
            { &small_.buf[0], &small_.buf[small_.len] } :
        std::pair<const CU*, const CU*>
            { large_.beg, large_.end };
}

template<typename CS>
CodePointRange<CS>::CodePointRange(const CodeUnit *beg, const CodeUnit *end)
    : beg_(beg), end_(end)
{
    AGZ_ASSERT(beg <= end);
}

template<typename CS>
CodePointRange<CS>::CodePointRange(const String<CS> &str, const CodeUnit *beg,
                                                          const CodeUnit *end)
    : str_(str)
{
    AGZ_ASSERT(str.begin() <= beg && beg <= end && end <= str.end());
    auto base = str.begin();
    beg_ = str_.begin() + (beg - base);
    end_ = str_.end()   + (end - base);
}

template<typename CS>
typename CodePointRange<CS>::Iterator CodePointRange<CS>>::begin() const
{
    return Iterator(beg_);
}

template<typename CS>
typename CodePointRange<CS>::Iterator CodePointRange<CS>::end() const
{
    return Iterator(end_);
}

template<typename CS>
StringView<CS>::CharRange::Iterator::Iterator(const String<CS> &str, InIt it)
    : str_(str), it_(it)
{

}

template<typename CS>
StringView<CS> StringView<CS>::CharRange::Iterator::operator*() const
{
    auto [b, e] = CS::CodeUnitsFromCodePointIterator(it);
    return StringView<CS>(str_, b, e - b);
}

template<typename CS>
typename StringView<CS>::CharRange::Iterator::pointer
StringView<CS>::CharRange::Iterator::operator->() const
{
    return pointer(**this);
}

template<typename CS>
typename StringView<CS>::CharRange::Iterator &
StringView<CS>::CharRange::Iterator::operator++()
{
    ++it;
    return *this;
}

template<typename CS>
typename StringView<CS>::CharRange::Iterator
StringView<CS>::CharRange::Iterator::operator++(int)
{
    auto ret = *this;
    ++*this;
    return ret;
}

template<typename CS>
typename StringView<CS>::CharRange::Iterator &
StringView<CS>::CharRange::Iterator::operator--()
{
    --it;
    return *this;
}

template<typename CS>
typename StringView<CS>::CharRange::Iterator
StringView<CS>::CharRange::Iterator::operator--(int)
{
    auto ret = *this;
    --*this;
    return ret;
}

template<typename CS>
bool StringView<CS>::CharRange::Iterator::operator==(const Self &rhs) const
{
    AGZ_ASSERT(&str_ == &rhs.str_);
    return it_ == rhs.it_;
}

template<typename CS>
bool StringView<CS>::CharRange::Iterator::operator!=(const Self &rhs) const
{
    return !(*this == rhs);
}

template<typename CS>
StringView<CS>::CharRange::CharRange(const CodeUnit *beg, const CodeUnit *end)
    : CPR_(beg, end)
{

}

template<typename CS>
StringView<CS>::CharRange::CharRange(const String<CS> &str, const CodeUnit *beg,
                                                            const CodeUnit *end)
    : CPR_(str, beg, end)
{

}

template<typename CS>
typename StringView<CS>::CharRange::Iterator
StringView<CS>::CharRange::begin() const
{
    return Iterator(CPR_.GetStr(), CPR_.begin());
}

template<typename CS>
typename StringView<CS>::CharRange::Iterator
StringView<CS>::CharRange::end() const
{
    return Iterator(CPR_.GetStr(), CPR_.end());
}

template<typename CS>
StringView<CS>::StringView(const Str &str)
    : str_(&str), beg_(str.Data()), len_(str.Length())
{

}

template<typename CS>
StringView<CS>::StringView(const Str &str, size_t begIdx, size_t endIdx)
    : str_(&str)
{
    AGZ_ASSERT(begIdx <= endIdx);
    AGZ_ASSERT(endIdx <= str.Length());
    auto d = str.Data();
    beg_   = d + begIdx;
    len_   = endIdx - begIdx;
}

template<typename CS>
StringView<CS>::StringView(const Str &str, const CodeUnit* beg, size_t len)
    : str_(&str), beg_(beg), len_(len)
{
    AGZ_ASSERT(beg_ >= str.Data() && beg_ + len_ <= str.Data() + str.Length());
}

template<typename CS>
String<CS> StringView<CS>::AsString() const
{
    if(beg_ == str_->Data() && len_ == str_->Length())
        return *str_;
    return Str(beg_, len_);
}

template<typename CS>
const typename CS::CodeUnit *StringView<CS>::Data() const
{
    return beg_;
}

template<typename CS>
std::pair<const typename CS::CodeUnit*, size_t>
StringView<CS>::DataAndLength() const
{
    return { beg_, len_ };
}

template<typename CS>
size_t StringView<CS>::Length() const
{
    return len_;
}

template<typename CS>
bool StringView<CS>::Empty() const
{
    return Length() == 0;
}

template<typename CS>
StringView<CS> StringView<CS>::Trim() const
{
    return TrimLeft().TrimRight();
}

template<typename CS>
StringView<CS> StringView<CS>::TrimLeft() const
{
    auto beg = beg_; auto len = len_;
    while(len > 0 && CS::IsSpace(*beg))
        ++beg, --len;
    return Self(*str_, beg, len);
}

template<typename CS>
StringView<CS> StringView<CS>::TrimRight() const
{
    auto len = len_;
    while(len > 0 && CS::IsSpace(beg_[len - 1]))
        --len;
    return Self(*str_, beg_, len);
}

template<typename CS>
StringView<CS> StringView<CS>::Slice(size_t begIdx) const
{
    return Slice(begIdx, len_);
}

template<typename CS>
StringView<CS> StringView<CS>::Slice(size_t begIdx, size_t endIdx) const
{
    AGZ_ASSERT(begIdx <= endIdx && endIdx <= len_);
    size_t idxOffset = beg_ - str_->begin();
    return Self(*str_, idxOffset + begIdx, idxOffset + endIdx);
}

template<typename CS>
StringView<CS> StringView<CS>::Prefix(size_t n) const
{
    AGZ_ASSERT(n <= len_);
    return Slice(0, n);
}

template<typename CS>
StringView<CS> StringView<CS>::Suffix(size_t n) const
{
    AGZ_ASSERT(n <= len_);
    return Slice(len_ - n);
}

template<typename CS>
bool StringView<CS>::StartsWith(const Self &prefix) const
{
    return len_ < prefix.Length() ?
                false :
                (Prefix(prefix.Length()) == prefix);
}

template<typename CS>
bool StringView<CS>::EndsWith(const Self &suffix) const
{
    return len_ < suffix.Length() ?
                false :
                (Suffix(suffix.Length()) == suffix);
}

// < 10    : digit
// [10, 36): alpha
// 128     : whitespaces
// 255     : others
inline const unsigned char DIGIT_CHAR_VALUE_TABLE[128] =
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 128,
    128, 128, 128, 128, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 128, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 0,   1,
    2,   3,   4,   5,   6,   7,   8,   9,   255, 255,
    255, 255, 255, 255, 255, 10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
    25,  26,  27,  28,  29,  30,  31,  32,  33,  34,
    35,  255, 255, 255, 255, 255, 255, 10,  11,  12,
    13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
    23,  24,  25,  26,  27,  28,  29,  30,  31,  32,
    33,  34,  35,  255, 255, 255, 255, 255
};

template<typename CS>
bool StringView<CS>::IsDigit(unsigned int base) const
{
    AGZ_ASSERT(base <= 36);
    auto [d, l] = DataAndLength();
    return l == 1 && d[0] < 128 &&
           DIGIT_CHAR_VALUE_TABLE[d[0]] < base;
}

template<typename CS>
bool StringView<CS>::IsDigits(unsigned int base) const
{
    AGZ_ASSERT(base <= 36);
    auto [d, l] = DataAndLength();
    for(size_t i = 0; i < l; ++i)
    {
        if(d[i] >= 128 || d[i] >= base)
            return false;
    }
    return true;
}

template<typename CS>
bool StringView<CS>::IsAlpha() const
{
    auto [d, l] = DataAndLength();
    return l == 1 && (('a' <= d[0] && d[0] <= 'z') ||
                      ('A' <= d[0] && d[0] <= 'Z'));
}

template<typename CS>
bool StringView<CS>::IsAlphas() const
{
    auto [d, l] = DataAndLength();
    for(size_t i = 0; i < l; ++i)
    {
        if(!(('a' <= d[i] && d[i] <= 'z') || ('A' <= d[i] && d[i] <= 'Z')))
            return false;
    }
    return true;
}

template<typename CS>
bool StringView<CS>::IsAlnum(unsigned int base) const
{
    auto [d, l] = DataAndLength();
    return l == 1 && d[0] < 128 && DIGIT_CHAR_VALUE_TABLE[d[0]] < 36;
}

template<typename CS>
bool StringView<CS>::IsAlnums(unsigned int base) const
{
    auto [d, l] = DataAndLength();
    for(size_t i = 0; i < l; ++i)
    {
        if(d[i] >= 128 || DIGIT_CHAR_VALUE_TABLE[d[i]] >= 36)
            return false;
    }
    return true;
}

template<typename CS>
bool StringView<CS>::IsUpper() const
{
    auto [d, l] = DataAndLength();
    return l == 1 && 'A' <= d[0] && d[0] <= 'Z';
}

template<typename CS>
bool StringView<CS>::IsUppers() const
{
    auto [d, l] = DataAndLength();
    for(size_t i = 0; i < l; ++i)
    {
        if(d[i] < 'A' || d[i] > 'Z')
            return false;
    }
    return true;
}

template<typename CS>
bool StringView<CS>::IsLower() const
{
    auto [d, l] = DataAndLength();
    return l == 1 && 'a' <= d[0] && d[0] <= 'z';
}

template<typename CS>
bool StringView<CS>::IsLowers() const
{
    auto [d, l] = DataAndLength();
    for(size_t i = 0; i < l; ++i)
    {
        if(d[i] < 'a' || d[i] > 'z')
            return false;
    }
    return true;
}

template<typename CS>
bool StringView<CS>::IsWhitespace() const
{
    auto [d, l] = DataAndLength();
    return l == 1 && d[0] < 128 && DIGIT_CHAR_VALUE_TABLE[d[0]] == 128;
}

template<typename CS>
bool StringView<CS>::IsWhitespaces() const
{
    auto [d, l] = DataAndLength();
    for(size_t i = 0; i < l; ++i)
    {
        if(d[i] >= 128 || DIGIT_CHAR_VALUE_TABLE[d[i]] != 128)
            return false;
    }
    return true;
}

template<typename CS>
bool StringView<CS>::IsASCII() const
{
    auto [d, l] = DataAndLength();
    for(size_t i = 0; i < l; ++i)
    {
        if(d[i] >= 128)
            return false;
    }
    return true;
}

template<typename CS>
String<CS> StringView<CS>::ToUpper() const
{
    String<CS> ret = *this;
    auto [b, e] = ret.BeginAndEnd();
    while(b < e)
    {
        if('a' <= *b && *b <= 'z')
        {
            *b = *b + ('A' - 'a');
            ++b;
        }
        else
            b = CS::NextCodePoint(b);
    }
    return std::move(ret);
}

template<typename CS>
String<CS> StringView<CS>::ToLower() const
{
    String<CS> ret = *this;
    auto [b, e] = ret.BeginAndEnd();
    while(b < e)
    {
        if('A' <= *b && *b <= 'Z')
        {
            *b = *b + ('a' - 'A');
            ++b;
        }
        else
            b = CS::NextCodePoint(b);
    }
    return std::move(ret);
}

template<typename CS>
String<CS> StringView<CS>::SwapCase() const
{
    String<CS> ret = *this;
    auto [b, e] = ret.BeginAndEnd();
    while(b < e)
    {
        if('a' <= *b && *b <= 'z')
        {
            *b = *b + ('A' - 'a');
            ++b;
        }
        else if('A' <= *b && *b <= 'Z')
        {
            *b = *b + ('a' - 'A');
            ++b;
        }
        else
            b = CS::NextCodePoint(b);
    }
    return std::move(ret);
}

template<typename CS>
std::vector<StringView<CS>> StringView<CS>::Split() const
{
    std::vector<Self> ret;
    const CodeUnit *segBeg = nullptr; size_t segLen = 0;
    for(auto p = beg_, e = beg_ + len_; p < e; ++p)
    {
        if(CS::IsSpace(*p))
        {
            if(segLen)
            {
                ret.emplace_back(*str_, segBeg, segLen);
                segLen = 0;
            }
        }
        else if(!segLen++)
            segBeg = p;
    }
    return std::move(ret);
}

template<typename CS>
std::vector<StringView<CS>>
StringView<CS>::Split(const Self &spliter) const
{
    AGZ_ASSERT(spliter.Empty() == false);
    std::vector<Self> ret;
    size_t segBeg = 0;
    while(segBeg < len_)
    {
        size_t fi = Find(spliter, segBeg);
        if(fi == NPOS)
        {
            ret.emplace_back(*str_, segBeg, len_);
            return std::move(ret);
        }

        if(fi != segBeg)
            ret.emplace_back(*str_, segBeg, fi);
        segBeg = fi + spliter.Length();
    }
    return std::move(ret);
}

template<typename CS>
template<typename R>
String<CS> StringView<CS>::Join(R &&strRange) const
{
    if(strRange.empty())
        return Str();
    StringBuilder<CS> builder;
    auto beg = std::begin(strRange), end = std::end(strRange);
    builder.Append(*beg++);
    while(beg != end)
        builder.Append(*this).Append(*beg++);
    return builder.Get();
}

template<typename CS>
size_t StringView<CS>::Find(const Self &dst, size_t begIdx) const
{
    AGZ_ASSERT(begIdx <= len_);
    auto rt = StrAlgo::FindSubPattern(begin() + begIdx, end(),
                                      dst.begin(), dst.end());
    return rt == end() ? NPOS : (rt - beg_);
}

template<typename CS>
typename StringView<CS>::Iterator StringView<CS>::begin() const
{
    return beg_;
}

template<typename CS>
typename StringView<CS>::Iterator StringView<CS>::end() const
{
    return beg_ + len_;
}

template<typename CS>
std::string StringView<CS>::ToStdString(NativeCharset cs) const
{
    switch(cs)
    {
    case NativeCharset::UTF8:
        if constexpr(std::is_same_v<CS, UTF8<char>>)
        {
            auto [d, l] = DataAndLength();
            return std::string(d, l);
        }
        else
        {
            return CharsetConvertor::Convert<UTF8<>, CS>(*this)
                        .ToStdString(NativeCharset::UTF8);
        }
    }
    throw CharsetException("Unknown charset " +
            std::to_string(static_cast<
                std::underlying_type_t<NativeCharset>>(cs)));
}

template<typename CS>
String<CS> StringView<CS>::operator+(const Self &rhs) const
{
    StringBuilder<CS> builder;
    builder << *this << rhs;
    return builder.Get();
}

template<typename CS>
bool StringView<CS>::operator==(const Self &rhs) const
{
    if(len_ != rhs.len_)
        return false;
    for(size_t i = 0; i < len_; ++i)
    {
        if(beg_[i] != rhs.beg_[i])
            return false;
    }
    return true;
}

template<typename CS>
bool StringView<CS>::operator!=(const Self &rhs) const
{
    return !(*this == rhs);
}

template<typename CS>
bool StringView<CS>::operator<(const Self &rhs) const
{
    return StrAlgo::Compare(beg_, rhs.beg_, len_, rhs.len_)
        == StrAlgo::CompareResult::Less;
}

template<typename CS>
bool StringView<CS>::operator>(const Self &rhs) const
{
    return StrAlgo::Compare(beg_, rhs.beg_, len_, rhs.len_)
        == StrAlgo::CompareResult::Greater;
}

template<typename CS>
bool StringView<CS>::operator<=(const Self &rhs) const
{
    return StrAlgo::Compare(beg_, rhs.beg_, len_, rhs.len_)
        != StrAlgo::CompareResult::Greater;
}

template<typename CS>
bool StringView<CS>::operator>=(const Self &rhs) const
{
    return StrAlgo::Compare(beg_, rhs.beg_, len_, rhs.len_)
        != StrAlgo::CompareResult::Less;
}

template<typename CS>
typename CS::CodeUnit *String<CS>::GetMutableData()
{
    return storage_.GetMutableData();
}

template<typename CS>
String<CS>::String(size_t len)
    : storage_(len)
{

}

template<typename CS>
String<CS>::String()
    : storage_(0)
{

}

template<typename CS>
String<CS>::String(const View &view)
    : String(view.AsString())
{

}

template<typename CS>
String<CS>::String(const CodeUnit *beg, size_t len)
    : storage_(beg, len)
{

}

template<typename CS>
String<CS>::String(const CodeUnit *beg, const CodeUnit *end)
    : storage_(beg, end)
{

}

template<typename CS>
String<CS>::String(const Self &copyFrom, size_t begIdx, size_t endIdx)
    : storage_(copyFrom.storage_, begIdx, endIdx)
{

}

template<typename CS>
String<CS>::String(const char *cstr, NativeCharset cs)
    : storage_(0)
{
    switch(cs)
    {
    case NativeCharset::UTF8:
        *this = Self(CharsetConvertor::Convert<CS, UTF8<>>(
                        Str8(cstr, std::strlen(cstr))));
        break;
    default:
        throw CharsetException("Unknown charset " +
            std::to_string(static_cast<
                std::underlying_type_t<NativeCharset>>(cs)));
    }
}

template<typename CS>
String<CS>::String(const std::string & cppStr, NativeCharset cs)
    : storage_(0)
{
    switch(cs)
    {
    case NativeCharset::UTF8:
        *this = Self(CharsetConvertor::Convert<CS, UTF8<>>(
                        Str8(cppStr.c_str(), cppStr.length())));
        break;
    default:
        throw CharsetException("Unknown charset " +
            std::to_string(static_cast<
                std::underlying_type_t<NativeCharset>>(cs)));
    }
}

template<typename CS>
String<CS>::String(const Self &copyFrom)
    : storage_(copyFrom.storage_)
{

}

template<typename CS>
String<CS>::String(Self &&moveFrom) noexcept
    : storage_(std::move(moveFrom.storage_))
{

}

template<typename CS>
String<CS> &String<CS>::operator=(const Self &copyFrom)
{
    storage_ = copyFrom.storage_;
    return *this;
}

template<typename CS>
String<CS> &String<CS>::operator=(Self &&moveFrom) noexcept
{
    storage_ = std::move(moveFrom.storage_);
    return *this;
}

#define AGZ_STR_FROM_INT_IMPL(T) \
    template<typename CS> \
    String<CS> String<CS>::From(T v, unsigned int base) \
    { return StrAlgo::Int2Str<T, CS>(v, base); }

AGZ_STR_FROM_INT_IMPL(char)
AGZ_STR_FROM_INT_IMPL(signed char)
AGZ_STR_FROM_INT_IMPL(unsigned char)
AGZ_STR_FROM_INT_IMPL(short)
AGZ_STR_FROM_INT_IMPL(unsigned short)
AGZ_STR_FROM_INT_IMPL(int)
AGZ_STR_FROM_INT_IMPL(unsigned int)
AGZ_STR_FROM_INT_IMPL(long)
AGZ_STR_FROM_INT_IMPL(unsigned long)
AGZ_STR_FROM_INT_IMPL(long long)
AGZ_STR_FROM_INT_IMPL(unsigned long long)

#undef AGZ_STR_FROM_INT_IMPL

template<typename CS>
StringView<CS> String<CS>::AsView() const
{
    return View(*this);
}

template<typename CS>
const typename CS::CodeUnit *String<CS>::Data() const
{
    return storage_.Begin();
}

template<typename CS>
std::pair<const typename CS::CodeUnit*, size_t>
String<CS>::DataAndLength() const
{
    return storage_.BeginAndLength();
}

template<typename CS>
size_t String<CS>::Length() const
{
    return storage_.GetLength();
}

template<typename CS>
bool String<CS>::Empty() const
{
    return Length() == 0;
}

template<typename CS>
typename String<CS>::Iterator String<CS>::begin() const
{
    return storage_.Begin();
}

template<typename CS>
typename String<CS>::Iterator String<CS>::end() const
{
    return storage_.End();
}

template<typename CS>
String<CS> operator*(const String<CS> &lhs, size_t rhs)
{
    StringBuilder<CS> b;
    b.Append(lhs, rhs);
    return b.Get();
}

template<typename CS>
StringBuilder<CS> &StringBuilder<CS>::Append(
    const StringView<CS> &view, size_t n)
{
    // IMPROVE
    while(n-- > 0)
        strs_.emplace_back(view);
    return *this;
}

template<typename CS>
StringBuilder<CS> &StringBuilder<CS>::Append(const String<CS> &str, size_t n)
{
    // IMPROVE
    while(n-- > 0)
        strs_.emplace_back(str);
    return *this;
}

template<typename CS>
StringBuilder<CS> &StringBuilder<CS>::operator<<(const StringView<CS> &view)
{
    return Append(view);
}

template<typename CS>
String<CS> StringBuilder<CS>::Get() const
{
    if(strs_.size() == 1)
        return strs_.front();

    size_t len = 0;
    for(auto &s : strs_)
        len += s.Length();

    String<CS> ret(len);
    auto data = ret.GetMutableData();
    for(auto &s : strs_)
    {
        Copy(s.Data(), s.Length(), data);
        data += s.Length();
    }

    strs_.clear();
    if(!ret.Empty())
        strs_.push_back(ret);
    return std::move(ret);
}

template<typename CS>
void StringBuilder<CS>::Clear()
{
    strs_.clear();
}

template<typename DCS, typename SCS>
String<DCS> CharsetConvertor::Convert(const typename String<SCS>::View &src)
{
    if constexpr(std::is_same_v<DCS, SCS>)
        return String<SCS>(src.AsString());
    else
    {
        std::vector<typename DCS::CodeUnit> cus;
        typename DCS::CodeUnit sgl[DCS::MaxCUInCP];

        auto beg = src.begin(), end = src.end();
        while(beg < end)
        {
            typename SCS::CodePoint scp;
            size_t skip = SCS::CU2CP(beg, &scp);
            if(!skip)
                throw CharsetException("Invalid " + SCS::Name() + " sequence");
            beg += skip;

            size_t sgls = DCS::CP2CU(DCS::template From<SCS>(scp), sgl);
            AGZ_ASSERT(sgls);
            for(size_t i = 0; i < sgls; ++i)
                cus.push_back(sgl[i]);
        }

        return String<DCS>(cus.data(), cus.size());
    }
}

AGZ_NS_END(AGZ::StrImpl)
