#include <algorithm>

#include "../Misc/Malloc.h"
#include "../Range/Reverse.h"
#include "StrAlgo.h"

AGZ_NS_BEG(AGZ::StrImpl)

template<typename CU>
void Storage<CU>::AllocSmall(size_t len)
{
    AGZ_ASSERT(len <= SMALL_BUF_SIZE);
    small_.len = len;
}

template<typename E>
RefCountedBuf<E> *RefCountedBuf<E>::New(size_t n)
{
    size_t allocSize = (sizeof(Self) - sizeof(E)) + n * sizeof(E);
    Self *ret = alloc_throw(std::malloc, allocSize);
    ret->refs_ = 1;
    return ret;
}

template<typename E>
void RefCountedBuf<E>::IncRef() const
{
    ++refs_;
}

template<typename E>
void RefCountedBuf<E>::DecRef() const
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
    small_.len = len;
}

template<typename CU>
void Storage<CU>::AllocLarge(size_t len)
{
    small_.len = SMALL_BUF_SIZE + 1;
    large_.buf = LargeBuf::New(len);
    large_.beg = large_.buf.GetData();
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
    return large_.beg;
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
        AllocSmall();
    else
        AllocLarge();
}

template<typename CU>
Storage<CU>::Storage(const CU *data, size_t len)
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

template<typename CU>
Storage<CU>::Storage(Self &&moveFrom)
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
Storage<CU> &Storage<CU>::operator=(Self &&moveFrom)
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
    return return large_.end - large_.beg;
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
                { &small_.buf[0], small_.len } :
                { large_.beg, large_.end - large_.beg };
}

template<typename CU>
std::pair<const CU*, size_t> Storage<CU>::BeginAndEnd() const
{
    return IsSmallStorage() ?
                { &small_.buf[0], &small_.buf[small_.len] } :
                { large_.beg, large_.end };
}

template<typename CS>
String<CS>::View::View(const Str &str)
    : str_(&str)
{
    std::tie(beg_, len_) = str.DataAndLength();
}

template<typename CS>
String<CS>::View::View(const Str &str, size_t begIdx, size_t endIdx)
    : str_(&str)
{
    AGZ_ASSERT(begIdx <= endIdx);
    AGZ_ASSERT(endIdx <= str.Length());
    auto d = str.Data();
    beg_   = d + begIdx;
    len_   = endIdx - begIdx;
}

template<typename CS>
String<CS>::View::View(const Str &str, const CodeUnit* beg, size_t len)
    : str_(&str), beg_(beg), len_(len)
{
    AGZ_ASSERT(beg_ >= str.Data() && beg_ + len_ <= str.Data() + str.Length());
}

template<typename CS>
const String<CS> String<CS>::View::AsString() const
{
    if(beg_ == str_->Data() && len_ == str_->Length())
        return *str_;
    return Str(beg_, len_);
}

template<typename CS>
const typename CS::CodeUnit *String<CS>::View::Data() const
{
    return beg_;
}

template<typename CS>
std::pair<const typename CS::CodeUnit*, size_t>
String<CS>::View::DataAndLength() const
{
    return { beg_, len_ };
}

template<typename CS>
size_t String<CS>::View::Length() const
{
    return len_;
}

template<typename CS>
bool String<CS>::View::Empty() const
{
    return Length() == 0;
}

template<typename CS>
typename String<CS>::View String<CS>::View::Trim() const
{
    return TrimLeft().TrimRight();
}

template<typename CS>
typename String<CS>::View String<CS>::View::TrimLeft() const
{
    auto beg = beg_; auto len = len_;
    while(len > 0 && CS::IsSpace(*beg))
        ++beg, --len;
    return Self(*str_, beg, len);
}

template<typename CS>
typename String<CS>::View String<CS>::View::TrimRight() const
{
    auto len = len_;
    while(len > 0 && CS::IsSpace(beg_[len - 1]))
        --len;
    return Self(*str_, beg_, len);
}

template<typename CS>
typename String<CS>::View String<CS>::View::Slice(size_t begIdx) const
{
    return Slice(begIdx, len_);
}

template<typename CS>
typename String<CS>::View
String<CS>::View::Slice(size_t begIdx, size_t endIdx) const
{
    AGZ_ASSERT(begIdx <= endIdx && endIdx <= len_);
    return Self(*str_, beg_ + begIdx, beg_ + endIdx);
}

template<typename CS>
typename String<CS>::View String<CS>::View::Prefix(size_t n) const
{
    AGZ_ASSERT(n <= len_);
    return Slice(0, n);
}

template<typename CS>
typename String<CS>::View String<CS>::View::Suffix(size_t n) const
{
    AGZ_ASSERT(n <= len_);
    return Slice(len_ - n);
}

template<typename CS>
bool String<CS>::View::StartsWith(const Self &prefix) const
{
    return len_ < prefix.Length() ?
                false :
                (Prefix(prefix.Length()) == prefix);
}

template<typename CS>
bool String<CS>::View::EndsWith(const Self &suffix) const
{
    return len_ < suffix.Length() ?
                false :
                (Suffix(suffix.Length()) == suffix);
}

template<typename CS>
std::vector<typename String<CS>::View> String<CS>::View::Split() const
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
std::vector<typename String<CS>::View>
String<CS>::View::Split(const Self &spliter) const
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
String<CS> String<CS>::Join(R &&strRange) const
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
size_t String<CS>::View::Find(const Self &dst, size_t begIdx) const
{
    AGZ_ASSERT(begIdx <= len_);
    auto rt = FindSubPattern(begin() + begIdx, end(), dst.begin(), dst.end());
    return rt == end() ? NPOS : (rt - beg_)
}

template<typename CS>
size_t String<CS>::View::FindR(const Self &dst, size_t rbegIdx) const
{
    AGZ_ASSERT(rbegIdx <= len_);
    using R = ReverseIterator<Iterator>;
    auto rbeg = R(end()) + rbegIdx, rend = R(begin());
    auto rt = FindSubPattern(rbeg, rend, R(dst.end()), R(dst.begin()));
    return rt == rend ? NPOS : (len_ - (rt - rbeg) - dst.Length());
}

template<typename CS>
typename String<CS>::View::Iterator String<CS>::View::begin() const
{
    return beg_;
}

template<typename CS>
typename String<CS>::View::Iterator String<CS>::View::end() const
{
    return beg_ + len_;
}

template<typename CS>
std::string String<CS>::View::ToStdString(NativeCharset cs) const
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
            return StringConvertor::Convert<UTF8<>>(*this)
                        .ToStdString(NativeCharset::UTF8);
        }
        break;
    }
    throw CharsetException("Unknown charset " +
            std::to_string(static_cast<
                std::underlying_type_t<NativeCharset>>(cs)));
}

template<typename CS>
bool String<CS>::View::operator==(const Self &rhs) const
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
bool String<CS>::View::operator==(const Self &rhs) const
{
    return !(*this == rhs);
}

template<typename CS>
bool String<CS>::View::operator<(const Self &rhs) const
{
    return StrAlgo::Compare(beg_, rhs.beg_, len_, rhs.len_)
        == StrAlgo::CompareResult::Less;
}

template<typename CS>
bool String<CS>::View::operator>(const Self &rhs) const
{
    return StrAlgo::Compare(beg_, rhs.beg_, len_, rhs.len_)
        == StrAlgo::CompareResult::Greater;
}

template<typename CS>
bool String<CS>::View::operator<=(const Self &rhs) const
{
    return StrAlgo::Compare(beg_, rhs.beg_, len_, rhs.len_)
        != StrAlgo::CompareResult::Greater;
}

template<typename CS>
bool String<CS>::View::operator>=(const Self &rhs) const
{
    return StrAlgo::Compare(beg_, rhs.beg_, len_, rhs.len_)
        != StrAlgo::CompareResult::Less;
}

template<typename CS>
String<CS>::String()
    : storage_(0)
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
String<CS>::String(const Self &copyFrom)
    : storage_(copyFrom.storage_)
{

}

template<typename CS>
String<CS>::String(Self &&moveFrom)
    : storage_(std::move(moveFrom.storage_))
{

}

template<typename CS>
String<CS>::operator View() const
{
    return AsView();
}

template<typename CS>
String<CS> &String<CS>::operator=(const Self &copyFrom)
{
    storage_ = copyFrom.storage_;
    return *this;
}

template<typename CS>
String<CS> &String<CS>::operator=(Self &&moveFrom)
{
    storage_ = std::move(moveFrom.storage_);
    return *this;
}

template<typename CS>
typename String<CS>::View String<CS>::AsView() const
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
typename String<CS>::View String<CS>::Trim() const
{
    return AsView().Trim();
}

template<typename CS>
typename String<CS>::View String<CS>::TrimLeft() const
{
    return AsView().TrimLeft();
}

template<typename CS>
typename String<CS>::View String<CS>::TrimRight() const
{
    return AsView().TrimRight();
}

template<typename CS>
typename String<CS>::View String<CS>::Slice(size_t begIdx) const
{
    return AsView().Slice(begIdx);
}

template<typename CS>
typename String<CS>::View String<CS>::Slice(size_t begIdx, size_t endIdx) const
{
    return AsView().Slice(begIdx, endIdx);
}

template<typename CS>
typename String<CS>::View String<CS>::Prefix(size_t n) const
{
    return AsView().Prefix(n);
}

template<typename CS>
typename String<CS>::View String<CS>::Suffix(size_t n) const
{
    return AsView().Suffix(n);
}

template<typename CS>
bool String<CS>::StartsWith(const View &prefix) const
{
    return AsView().StartsWith(prefix);
}

template<typename CS>
bool String<CS>::EndsWith(const View &suffix) const
{
    return AsView().EndsWith(suffix);
}

template<typename CS>
std::vector<typename String<CS>::View> String<CS>::Split() const
{
    return AsView().Split();
}

template<typename CS>
std::vector<typename String<CS>::View>
String<CS>::Split(const View &spliter) const
{
    return AsView().Split(spliter);
}

template<typename CS>
template<typename R>
String<CS> String<CS>::Join(R &&strRange) const
{
    return AsView().Join(std::forward<R>(strRange))
}

template<typename CS>
size_t String<CS>::Find(const View &dst, size_t begIdx) const
{
    return AsView().Find(dst, begIdx);
}

template<typename CS>
size_t String<CS>::FindR(const View &dst, size_t rbegIdx) const
{
    return AsView().FindR(dst, rbegIdx);
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
std::string String<CS>::ToStdString() const
{
    return AsView().ToStdString();
}

template<typename CS>
StringBuilder<CS> &StringBuilder<CS>::Append(
    const typename String<CS>::View &view)
{
    strs_.emplace_back(view);
    return *this;
}

template<typename CS>
StringBuilder<CS> &StringBuilder<CS>::operator<<(
    const typename String<CS>::View &view)
{
    return Append(view);
}

template<typename CS>
String<CS> StringBuilder<CS>::Get() const
{
    size_t len = 0;
    for(auto &s : strs_)
        len += s.Length();
    String<CS> ret(len);
    auto data = ret.GetMutableData();
    for(auto &s : strs_)
    {
        std::copy_n(s.Data(), s.Length(), data);
        data += s.Length();
    }
    return std::move(ret);
}

template<typename CS>
void StringBuilder<CS>::Clear()
{
    strs_.clear();
}

template<typename DCS, typename SCS>
String<DCS> StringConvertor::Convert(const String<SCS>::View &src)
{
    if constexpr(std::is_same_v<DCS, SCS>)
        return String<SCS>(src.AsString());
    else
    {
        std::vector<typename DCS::CodeUnit> cus;
        CodeUnit sgl[DCS::MaxCUInCP];

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
