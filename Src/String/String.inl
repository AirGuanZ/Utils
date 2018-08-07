#include <algorithm>

#include "../Misc/Malloc.h"
#include "../Range/Reverse.h"
#include "StrAlgo.h"

AGZ_NS_BEG(AGZ::StrImpl)

template<typename CU, typename TP>
void Storage<CU, TP>::AllocSmall(size_t len)
{
    AGZ_ASSERT(len <= SMALL_BUF_SIZE);
    small_.len = len;
}

template<typename E, typename TP>
RefCountedBuf<E, TP> *RefCountedBuf<E, TP>::New(size_t n)
{
    size_t allocSize = (sizeof(Self) - sizeof(E)) + n * sizeof(E);
    Self *ret = alloc_throw(std::malloc, allocSize);
    ret->refs_ = 1;
    return ret;
}

template<typename E, typename TP>
void RefCountedBuf<E, TP>::IncRef() const
{
    ++refs_;
}

template<typename E, typename TP>
void RefCountedBuf<E, TP>::DecRef() const
{
    if(!--refs_)
        std::free(this);
}

template<typename E, typename TP>
E *RefCountedBuf<E, TP>::GetData()
{
    return &data_[0];
}

template<typename E, typename TP>
const E *RefCountedBuf<E, TP>::GetData() const
{
    return &data_[0];
}

template<typename CU, typename TP>
void Storage<CU, TP>::AllocSmall(size_t len)
{
    AGZ_ASSERT(len <= SMALL_BUF_SIZE);
    small_.len = len;
}

template<typename CU, typename TP>
void Storage<CU, TP>::AllocLarge(size_t len)
{
    small_.len = SMALL_BUF_SIZE + 1;
    large_.buf = LargeBuf::New(len);
    large_.beg = large_.buf.GetData();
    large_.end = large_.beg + len;
}

template<typename CU, typename TP>
CU *Storage<CU, TP>::GetSmallMutableData()
{
    AGZ_ASSERT(IsSmallStorage());
    return &small_.buf[0];
}

template<typename CU, typename TP>
CU *Storage<CU, TP>::GetLargeMutableData()
{
    AGZ_ASSERT(IsLargeStorage());
    return large_.beg;
}

template<typename CU, typename TP>
CU *Storage<CU, TP>::GetMutableData()
{
    return IsSmallStorage() ? GetSmallMutableData() :
                              GetLargeMutableData();
}

template<typename CU, typename TP>
Storage<CU, TP>::Storage(size_t len)
{
    if(len <= SMALL_BUF_SIZE)
        AllocSmall();
    else
        AllocLarge();
}

template<typename CU, typename TP>
Storage<CU, TP>::Storage(const CU *data, size_t len)
{
    if(len <= SMALL_BUF_SIZE)
    {
        AllocSmall();
        std::copy_n(data, len, GetSmallMutableData());
    }
    else
    {
        AllocLarge();
        std::copy_n(data, len, GetLargeMutableData());
    }
}

template<typename CU, typename TP>
Storage<CU, TP>::Storage(const CU *beg, const CU *end)
    : Storage(beg, static_cast<size_t>(end - beg))
{

}

template<typename CU, typename TP>
Storage<CU, TP>::Storage(const Self &copyFrom)
    : Storage(copyFrom, 0, copyFrom.GetLength())
{

}

template<typename CU, typename TP>
Storage<CU, TP>::Storage(const Self &copyFrom, size_t beg, size_t end)
{
    AGZ_ASSERT(beg <= end);
    size_t len = end - beg;
    if(len <= SMALL_BUF_SIZE)
    {
        AllocSmall();
        std::copy_n(&copyFrom.Begin()[beg], len, GetSmallMutableData());
    }
    else
    {
        small_.len = copyFrom.small_.len;
        large_.buf = copyFrom.large_.buf;
        large_.beg = &copyFrom.Begin()[beg];
        large_.end = large_.beg + len;
    }
}

template<typename CU, typename TP>
Storage<CU, TP>::Storage(Self &&moveFrom)
{
    if(moveFrom.IsLargeStorage())
    {
        small_.len = moveFrom.small_.len;
        large_ = moveFrom.large_;
        moveFrom.small_.len = 0;
    }
    else
    {
        auto [data, len] = moveFrom.DataAndLength();
        AllocSmall(len);
        std::copy_n(data, len, GetSmallMutableData());
    }
}

template<typename CU, typename TP>
Storage<CU, TP>::~Storage()
{
    if(IsLargeStorage())
        large_.buf->DecRef();
}

template<typename CU, typename TP>
Storage<CU, TP> &Storage<CU, TP>::operator=(const Self &copyFrom)
{
    if(IsLargeStorage())
        large_.buf->DecRef();
    new(this) Self(copyFrom);
    return *this;
}

template<typename CU, typename TP>
Storage<CU, TP> &Storage<CU, TP>::operator=(Self &&moveFrom)
{
    if(IsLargeStorage())
        large_.buf->DecRef();
    new(this) Self(std::move(moveFrom));
    return *this;
}

template<typename CU, typename TP>
bool Storage<CU, TP>::IsSmallStorage() const
{
    return small_.len <= SMALL_BUF_SIZE;
}

template<typename CU, typename TP>
bool Storage<CU, TP>::IsLargeStorage() const
{
    return !IsSmallStorage();
}

template<typename CU, typename TP>
size_t Storage<CU, TP>::GetSmallLength() const
{
    AGZ_ASSERT(IsSmallStorage());
    return small_.len;
}

template<typename CU, typename TP>
size_t Storage<CU, TP>::GetLargeLength() const
{
    AGZ_ASSERT(IsLargeStorage() && large_.beg <= large_.end);
    return return large_.end - large_.beg;
}

template<typename CU, typename TP>
size_t Storage<CU, TP>::GetLength() const
{
    return IsSmallStorage() ? GetSmallLength() : GetLargeLength();
}

template<typename CU, typename TP>
const CU *Storage<CU, TP>::Begin() const
{
    return IsSmallStorage() ? &small_.buf[0] : large_.beg;
}

template<typename CU, typename TP>
const CU *Storage<CU, TP>::End() const
{
    return IsSmallStorage() ? (&small_.buf[small_.len]) : large_.end;
}

template<typename CU, typename TP>
std::pair<const CU*, size_t> Storage<CU, TP>::BeginAndLength() const
{
    return IsSmallStorage() ?
                { &small_.buf[0], small_.len } :
                { large_.beg, large_.end - large_.beg };
}

template<typename CU, typename TP>
std::pair<const CU*, size_t> Storage<CU, TP>::BeginAndEnd() const
{
    return IsSmallStorage() ?
                { &small_.buf[0], &small_.buf[small_.len] } :
                { large_.beg, large_.end };
}

template<typename CS, typename TP>
String<CS, TP>::View::View(const Str &str)
    : str_(&str)
{
    std::tie(beg_, len_) = str.DataAndLength();
}

template<typename CS, typename TP>
String<CS, TP>::View::View(const Str &str, size_t begIdx, size_t endIdx)
    : str_(&str)
{
    AGZ_ASSERT(begIdx <= endIdx);
    AGZ_ASSERT(endIdx <= str.Length());
    auto d = str.Data();
    beg_   = d + begIdx;
    len_   = endIdx - begIdx;
}

template<typename CS, typename TP>
String<CS, TP>::View::View(const Str &str, const CodeUnit* beg, size_t len)
    : str_(&str), beg_(beg), len_(len)
{
    AGZ_ASSERT(beg_ >= str.Data() && beg_ + len_ <= str.Data() + str.Length());
}

template<typename CS, typename TP>
const typename CS::CodeUnit *String<CS, TP>::View::Data() const
{
    return beg_;
}

template<typename CS, typename TP>
std::pair<const typename CS::CodeUnit*, size_t>
String<CS, TP>::View::DataAndLength() const
{
    return { beg_, len_ };
}

template<typename CS, typename TP>
size_t String<CS, TP>::View::Length() const
{
    return len_;
}

template<typename CS, typename TP>
bool String<CS, TP>::View::Empty() const
{
    return Length() == 0;
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::View::Trim() const
{
    return TrimLeft().TrimRight();
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::View::TrimLeft() const
{
    auto beg = beg_; auto len = len_;
    while(len > 0 && CS::IsSpace(*beg))
        ++beg, --len;
    return Self(*str_, beg, len);
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::View::TrimRight() const
{
    auto len = len_;
    while(len > 0 && CS::IsSpace(beg_[len - 1]))
        --len;
    return Self(*str_, beg_, len);
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::View::Slice(size_t begIdx) const
{
    return this->Slice(begIdx, len_);
}

template<typename CS, typename TP>
typename String<CS, TP>::View
String<CS, TP>::View::Slice(size_t begIdx, size_t endIdx) const
{
    AGZ_ASSERT(begIdx <= endIdx && endIdx <= len_);
    return Self(*str_, beg_ + begIdx, beg_ + endIdx);
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::View::Prefix(size_t n) const
{
    AGZ_ASSERT(n <= len_);
    return this->Slice(0, n);
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::View::Suffix(size_t n) const
{
    AGZ_ASSERT(n <= len_);
    return this->Slice(len_ - n);
}

template<typename CS, typename TP>
bool String<CS, TP>::View::StartsWith(const Self &prefix) const
{
    return len_ < prefix.Length() ?
                false :
                (Prefix(prefix.Length()) == prefix);
}

template<typename CS, typename TP>
bool String<CS, TP>::View::EndsWith(const Self &suffix) const
{
    return len_ < suffix.Length() ?
                false :
                (Suffix(suffix.Length()) == suffix);
}

template<typename CS, typename TP>
std::vector<typename String<CS, TP>::View> String<CS, TP>::View::Split() const
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

template<typename CS, typename TP>
std::vector<typename String<CS, TP>::View>
String<CS, TP>::View::Split(const Self &spliter) const
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

template<typename CS, typename TP>
template<typename R>
String<CS, TP> String<CS, TP>::Join(R &&strRange) const
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

template<typename CS, typename TP>
size_t String<CS, TP>::View::Find(const Self &dst, size_t begIdx) const
{
    AGZ_ASSERT(begIdx <= len_);
    auto rt = FindSubPattern(begin() + begIdx, end(), dst.begin(), dst.end());
    return rt == end() ? NPOS : (rt - beg_)
}

template<typename CS, typename TP>
size_t String<CS, TP>::View::FindR(const Self &dst, size_t rbegIdx) const
{
    AGZ_ASSERT(rbegIdx <= len_);
    using R = ReverseIterator<Iterator>;
    auto rbeg = R(end()) + rbegIdx, rend = R(begin());
    auto rt = FindSubPattern(rbeg, rend, R(dst.end()), R(dst.begin()));
    return rt == rend ? NPOS : (len_ - (rt - rbeg) - dst.Length());
}

template<typename CS, typename TP>
typename String<CS, TP>::View::Iterator String<CS, TP>::View::begin() const
{
    return beg_;
}

template<typename CS, typename TP>
typename String<CS, TP>::View::Iterator String<CS, TP>::View::end() const
{
    return beg_ + len_;
}

template<typename CS, typename TP>
bool String<CS, TP>::View::operator==(const Self &rhs) const
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

template<typename CS, typename TP>
bool String<CS, TP>::View::operator==(const Self &rhs) const
{
    return !(*this == rhs);
}

template<typename CS, typename TP>
bool String<CS, TP>::View::operator<(const Self &rhs) const
{
    return StrAlgo::Compare(beg_, rhs.beg_, len_, rhs.len_)
        == StrAlgo::CompareResult::Less;
}

template<typename CS, typename TP>
bool String<CS, TP>::View::operator>(const Self &rhs) const
{
    return StrAlgo::Compare(beg_, rhs.beg_, len_, rhs.len_)
        == StrAlgo::CompareResult::Greater;
}

template<typename CS, typename TP>
bool String<CS, TP>::View::operator<=(const Self &rhs) const
{
    return StrAlgo::Compare(beg_, rhs.beg_, len_, rhs.len_)
        != StrAlgo::CompareResult::Greater;
}

template<typename CS, typename TP>
bool String<CS, TP>::View::operator>=(const Self &rhs) const
{
    return StrAlgo::Compare(beg_, rhs.beg_, len_, rhs.len_)
        != StrAlgo::CompareResult::Less;
}

template<typename CS, typename TP>
String<CS, TP>::String()
    : storage_(0)
{

}

template<typename CS, typename TP>
String<CS, TP>::String(const CodeUnit *beg, size_t len)
    : storage_(beg, len)
{

}

template<typename CS, typename TP>
String<CS, TP>::String(const CodeUnit *beg, const CodeUnit *end)
    : storage_(beg, end)
{

}

template<typename CS, typename TP>
String<CS, TP>::String(const Self &copyFrom, size_t begIdx, size_t endIdx)
    : storage_(copyFrom.storage_, begIdx, endIdx)
{

}

template<typename CS, typename TP>
String<CS, TP>::String(const Self &copyFrom)
    : storage_(copyFrom.storage_)
{

}

template<typename CS, typename TP>
String<CS, TP>::String(Self &&moveFrom)
    : storage_(std::move(moveFrom.storage_))
{

}

template<typename CS, typename TP>
template<typename OTP, std::enable_if_t<!std::is_same_v<TP, OTP>, int>>
String<CS, TP>::String(const String<CS, OTP> &copyFrom)
{
    auto [data, len] = copyFrom.DataAndLength();
    new(this) Self(data, len);
}

template<typename CS, typename TP>
String<CS, TP> &String<CS, TP>::operator=(const Self &copyFrom)
{
    storage_ = copyFrom.storage_;
    return *this;
}

template<typename CS, typename TP>
String<CS, TP> &String<CS, TP>::operator=(Self &&moveFrom)
{
    storage_ = std::move(moveFrom.storage_);
    return *this;
}

template<typename CS, typename TP>
template<typename OTP, std::enable_if_t<!std::is_same_v<TP, OTP>, int>>
String<CS, TP> &String<CS, TP>::operator=(const String<CS, OTP> &copyFrom)
{
    this->~String();
    auto [data, len] = copyFrom.DataAndLength();
    new(this) Self(data, len);
    return *this;
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::AsView() const
{
    return View(*this);
}

template<typename CS, typename TP>
const typename CS::CodeUnit *String<CS, TP>::Data() const
{
    return storage_.Begin();
}

template<typename CS, typename TP>
std::pair<const typename CS::CodeUnit*, size_t>
String<CS, TP>::DataAndLength() const
{
    return storage_.BeginAndLength();
}

template<typename CS, typename TP>
size_t String<CS, TP>::Length() const
{
    return storage_.GetLength();
}

template<typename CS, typename TP>
bool String<CS, TP>::Empty() const
{
    return Length() == 0;
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::Trim() const
{
    return AsView().Trim();
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::View::TrimLeft() const
{
    return AsView().TrimLeft();
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::View::TrimRight() const
{
    return AsView().TrimRight();
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::View::Slice(size_t begIdx) const
{
    return AsView().Slice(begIdx);
}

template<typename CS, typename TP>
typename String<CS, TP>::View
String<CS, TP>::View::Slice(size_t begIdx, size_t endIdx) const
{
    return AsView().Slice(begIdx, endIdx);
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::View::Prefix(size_t n) const
{
    return AsView().Prefix(n);
}

template<typename CS, typename TP>
typename String<CS, TP>::View String<CS, TP>::View::Suffix(size_t n) const
{
    return AsView().Suffix(n);
}

template<typename CS, typename TP>
bool String<CS, TP>::View::StartsWith(const View &prefix) const
{
    return AsView().StartsWith(prefix);
}

template<typename CS, typename TP>
bool String<CS, TP>::View::EndsWith(const View &suffix) const
{
    return AsView().EndsWith(suffix);
}

template<typename CS, typename TP>
std::vector<typename String<CS, TP>::View> String<CS, TP>::View::Split() const
{
    return AsView().Split();
}

template<typename CS, typename TP>
std::vector<typename String<CS, TP>::View>
String<CS, TP>::View::Split(const View &spliter) const
{
    return AsView().Split(spliter);
}

template<typename CS, typename TP>
template<typename R>
String<CS, TP> String<CS, TP>::Join(R &&strRange) const
{
    return AsView().Join(std::forward<R>(strRange))
}

template<typename CS, typename TP>
size_t String<CS, TP>::View::Find(const View &dst, size_t begIdx) const
{
    return AsView().Find(dst, begIdx);
}

template<typename CS, typename TP>
size_t String<CS, TP>::View::FindR(const View &dst, size_t rbegIdx) const
{
    return AsView().FindR(dst, rbegIdx);
}

template<typename CS, typename TP>
template<typename TPs>
StringBuilder<CS, TP> &StringBuilder<CS, TP>::Append(
    const typename String<CS, TPs>::View &view)
{
    strs_.emplace_back(view);
    return *this;
}

template<typename CS, typename TP>
template<typename TPs>
StringBuilder<CS, TP> &StringBuilder<CS, TP>::operator<<(
    const typename String<CS, TPs>::View &view)
{
    return Append(view);
}

template<typename CS, typename TP>
template<typename TPs>
String<CS, TPs> StringBuilder<CS, TP>::Get() const
{
    size_t len = 0;
    for(auto &s : strs_)
        len += s.Length();
    String<CS, TPs> ret(len);
    auto data = ret.GetMutableData();
    for(auto &s : strs_)
    {
        std::copy_n(s.Data(), s.Length(), data);
        data += s.Length();
    }
    return std::move(ret);
}

template<typename CS, typename TP>
void StringBuilder<CS, TP>::Clear()
{
    strs_.clear();
}

AGZ_NS_END(AGZ::StrImpl)
