#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <limits>
#include <list>
#include <string>
#include <vector>

#include "../../Misc/Common.h"
#include "../../Misc/Config.h"
#include "../../Range/Iterator.h"
#include "../Charset/ASCII.h"
#include "../Charset/UTF.h"

AGZ_NS_BEG(AGZ::StrImpl)

// Charsets that can be used by c-style string and std::string
enum class NativeCharset
{
    UTF8, // for const char * / std::string
    WUTF, // for const wchar_t * / std::wstring
};

// Reference counted value container
// Used for large string storage
template<typename E>
class RefCountedBuf
{
#if defined(AGZ_THREAD_SAFE_STRING)
    std::atomic<size_t> refs_;
#else
    size_t refs_;
#endif
    E data_[1];

public:

    using Self = RefCountedBuf<E>;

    static Self *New(size_t n);

    RefCountedBuf()              = delete;
    RefCountedBuf(const Self&)   = delete;
    ~RefCountedBuf()             = delete;
    Self &operator=(const Self&) = delete;

    void IncRef();
    void DecRef();

    E *GetData();
    const E *GetData() const;
};

// Implementation of string storage.
// Small strings are stored in internal buffer (small_.buf),
// while larges ones are shared by a reference counted buffer (large_.buf).
// Immutable design due to safe consideration.
// Can only be modified by StringBuilder to initialize contents.
template<typename CU>
class Storage
{
    static constexpr size_t SMALL_BUF_SIZE = 31 / sizeof(CU);

    using LargeBuf = RefCountedBuf<CU>;

    union
    {
        struct
        {
            CU buf[SMALL_BUF_SIZE];
            std::uint8_t len;
        } small_;

        struct
        {
            LargeBuf *buf;
            const CU *beg;
            const CU *end;
        } large_;
    };

    void AllocSmall(size_t len);
    void AllocLarge(size_t len);

    CU *GetSmallMutableData();
    CU *GetLargeMutableData();

public:

    using Self = Storage<CU>;

    static_assert(std::is_trivially_copyable_v<CU>);

    CU *GetMutableData();

    explicit Storage(size_t len);
    Storage(const CU *data, size_t len);
    Storage(const CU *beg, const CU *end);

    Storage(const Self &copyFrom);
    Storage(const Self &copyFrom, size_t begIdx, size_t endIdx);
    Storage(Self &&moveFrom) noexcept;

    ~Storage();

    Self &operator=(const Self &copyFrom);
    Self &operator=(Self &&moveFrom) noexcept;

    bool IsSmallStorage() const;
    bool IsLargeStorage() const;

    size_t GetSmallLength() const;
    size_t GetLargeLength() const;
    size_t GetLength() const;

    const CU *Begin() const;
    const CU *End() const;

    std::pair<const CU*, size_t> BeginAndLength() const;
    std::pair<const CU*, const CU*> BeginAndEnd() const;
};

template<typename CU>
class Storage_NoSSO
{
    using Buf = RefCountedBuf<CU>;
    Buf *buf_;
    CU *beg_, *end_;

    void Alloc(size_t len);

public:

    using Self = Storage_NoSSO<CU>;

    static_assert(std::is_trivially_copyable_v<CU>);

    CU *GetMutableData();

    explicit Storage_NoSSO(size_t len);
    Storage_NoSSO(const CU *data, size_t len);
    Storage_NoSSO(const CU *beg, const CU *end);

    Storage_NoSSO(const Self &copyFrom);
    Storage_NoSSO(const Self &copyFrom, size_t begIdx, size_t endIdx);
    Storage_NoSSO(Self &&moveFrom) noexcept;

    ~Storage_NoSSO();

    Self &operator=(const Self &copyFrom);
    Self &operator=(Self &&moveFrom) noexcept;

    size_t GetLength() const;

    const CU *Begin() const;
    const CU *End() const;

    std::pair<const CU*, size_t> BeginAndLength() const;
    std::pair<const CU*, const CU*> BeginAndEnd() const;
};

template<typename CS, typename Eng>
class Regex;

template<typename CS>
class StringBuilder;

template<typename CS>
class String;

template<typename CS>
class CodePointRange
{
    String<CS> str_;
    const typename CS::CodeUnit *beg_;
    const typename CS::CodeUnit *end_;

public:

    using CodeUnit  = typename CS::CodeUnit;
    using CodePoint = typename CS::CodePoint;
    using Iterator  = GetIteratorType<CS>;

    CodePointRange(const CodeUnit *beg, const CodeUnit *end);
    CodePointRange(const String<CS> &str, const CodeUnit *beg,
                                          const CodeUnit *end);

    Iterator begin() const;
    Iterator end()   const;

    const String<CS> &GetStr() const { return str_; }

    size_t CodeUnitIndex(const Iterator &it) const
    {
        return CS::CodeUnitsBeginFromCodePointIterator(it) - beg_;
    }
};

template<typename CS>
// Immutable string slice
class StringView
{
    const String<CS> *str_;
    const typename CS::CodeUnit *beg_;
    size_t len_;

public:

    class CharRange
    {
        using InIt = GetIteratorType<CS>;

        CodePointRange<CS> CPR_;

    public:

        using CodeUnit  = typename CS::CodeUnit;
        using CodePoint = typename CS::CodePoint;

        class Iterator
        {
            InIt it_;

        public:

            using iterator_category = std::bidirectional_iterator_tag;
            using value_type        = String<CS>;
            using difference_type   =
                typename std::iterator_traits<InIt>::difference_type;
            using pointer           = ValuePointer<value_type>;
            using reference         = value_type&;

            using Self = Iterator;

            explicit Iterator(InIt it);

            value_type operator*() const;
            pointer operator->() const;

            Self &operator++();
            Self operator++(int);
            Self &operator--();
            Self operator--(int);

            bool operator==(const Self &rhs) const;
            bool operator!=(const Self &rhs) const;
        };

        CharRange(const CodeUnit *beg, const CodeUnit *end);
        CharRange(const String<CS> &str, const CodeUnit *beg,
                                         const CodeUnit *end);

        Iterator begin() const;
        Iterator end()   const;
    };

    using Charset = CS;
    using CodeUnit = typename CS::CodeUnit;
    using CodePoint = typename CS::CodePoint;
    using Self = StringView<CS>;
    using Str = String<CS>;

    using Iterator = const CodeUnit*;

    static constexpr size_t NPOS = std::numeric_limits<size_t>::max();

    StringView(const Str &str);
    StringView(const Str &str, const CodeUnit* beg, size_t len);
    StringView(const Str &str, size_t begIdx, size_t endIdx);

    StringView() = delete;
    StringView(const Self &) = default;
    ~StringView() = default;
    Self &operator=(const Self &) = default;

    Str AsString() const;

    const CodeUnit *Data() const;
    std::pair<const CodeUnit*, size_t> DataAndLength() const;

    size_t Length() const;
    bool Empty()    const;

    template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    T Parse(unsigned base = 10) const;

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    T Parse() const;

    Self Trim()      const;
    Self TrimLeft()  const;
    Self TrimRight() const;

    Self Slice(size_t begIdx)                const;
    Self Slice(size_t begIdx, size_t endIdx) const;

    Self Prefix(size_t n) const;
    Self Suffix(size_t n) const;

    bool StartsWith(const Self &s) const;
    bool StartsWith(const Str &s)  const { return StartsWith(s.AsView()); }

    bool EndsWith(const Self &s) const;
    bool EndsWith(const Str &s)  const { return EndsWith(s.AsView()); }

    bool IsDigit(unsigned int base = 10)  const;
    bool IsDigits(unsigned int base = 10) const;

    bool IsAlpha()  const;
    bool IsAlphas() const;

    bool IsAlnum(unsigned int base = 10)  const;
    bool IsAlnums(unsigned int base = 10) const;

    bool IsUpper()  const;
    bool IsUppers() const;

    bool IsLower()  const;
    bool IsLowers() const;

    bool IsWhitespace()  const;
    bool IsWhitespaces() const;

    bool IsASCII() const;

    Str ToUpper()  const;
    Str ToLower()  const;
    Str SwapCase() const;

    std::vector<Self> Split()                    const;
    std::vector<Self> Split(const Self &spliter) const;
    std::vector<Self> Split(const Str &spliter)  const { return Split(spliter.AsView()); }

    template<typename C, std::enable_if_t<!std::is_array_v<C>, int> = 0,
                         typename = std::void_t<decltype(std::declval<C>().begin())>>
    std::vector<Self> Split(const C &spliters) const;

    template<typename R>
    Str Join(R &&strRange) const;

    size_t Find(const Self &dst, size_t begIdx = 0) const;
    size_t Find(const Str &dst, size_t begIdx = 0)  const { return Find(dst.AsView(), begIdx); }

    template<typename F>
    size_t FindCPIf(F &&f) const;

    CodePointRange<CS> CodePoints() const &  { return CodePointRange<CS>(beg_, beg_ + len_); }
    CodePointRange<CS> CodePoints() const && { return CodePointRange<CS>(*str_, beg_, beg_ + len_); }

    CharRange Chars() const &  { return CharRange(beg_, beg_ + len_); }
    CharRange Chars() const && { return CharRange(*str_, beg_, beg_ + len_); }

    std::string ToStdString(NativeCharset cs = NativeCharset::UTF8)  const;
    std::wstring ToStdWString(NativeCharset cs = NativeCharset::WUTF) const;

#if defined(AGZ_OS_WIN32)
    std::wstring ToPlatformString() const { return ToStdWString(); }
#else
    std::string ToPlatformString() const { return ToStdString(); }
#endif

    Iterator begin() const;
    Iterator end()   const;

    Str operator+(const Self &rhs) const;

    bool operator==(const Self &rhs) const;
    bool operator!=(const Self &rhs) const;
    bool operator< (const Self &rhs) const;
    bool operator> (const Self &rhs) const;
    bool operator<=(const Self &rhs) const;
    bool operator>=(const Self &rhs) const;
};

template<typename CS>
class String
{
#if defined(AGZ_ENABLE_STRING_SSO)
    using InternalStorage = Storage<typename CS::CodeUnit>;
#else
    using InternalStorage = Storage_NoSSO<typename CS::CodeUnit>;
#endif

    friend class StringBuilder<CS>;
    friend class StringView<CS>;

    InternalStorage storage_;

    typename CS::CodeUnit *GetMutableData();
    std::pair<typename CS::CodeUnit*, typename CS::CodeUnit*>
        GetMutableBeginAndEnd();

    explicit String(size_t len);

public:

    using Charset   = CS;
    using CodeUnit  = typename CS::CodeUnit;
    using CodePoint = typename CS::CodePoint;
    using Builder   = StringBuilder<CS>;
    using View      = StringView<CS>;
    using Self      = String<CS>;

    using CharRange = typename View::CharRange;

    using Iterator = const CodeUnit*;

    static constexpr size_t NPOS = View::NPOS;

    String();
    explicit String(CodePoint cp, size_t count = 1);
    String(const View &view);
    String(const CodeUnit *beg, size_t len);
    String(const CodeUnit *beg, const CodeUnit *end);
    String(const Self &copyFrom, size_t begIdx, size_t endIdx);

    String(const char *cstr, NativeCharset cs = NativeCharset::UTF8);
    String(const std::string &cppStr, NativeCharset cs = NativeCharset::UTF8);

    String(const wchar_t *cstr, NativeCharset cs = NativeCharset::WUTF);
    String(const std::wstring &cppStr, NativeCharset cs = NativeCharset::WUTF);

    String(const Self &copyFrom);
    String(Self &&moveFrom) noexcept;

    template<typename OCS, std::enable_if_t<!std::is_same_v<CS, OCS>, int> = 0>
    explicit String(const StringView<OCS> &convertFrom);

    template<typename OCS, std::enable_if_t<!std::is_same_v<CS, OCS>, int> = 0>
    explicit String(const String<OCS> &convertFrom): Self(convertFrom.AsView()) {  }

    ~String() = default;

    Self &operator=(const Self &copyFrom);
    Self &operator=(Self &&moveFrom) noexcept;

    static Self From(char               v, unsigned int base = 10);
    static Self From(signed char        v, unsigned int base = 10);
    static Self From(unsigned char      v, unsigned int base = 10);
    static Self From(short              v, unsigned int base = 10);
    static Self From(unsigned short     v, unsigned int base = 10);
    static Self From(int                v, unsigned int base = 10);
    static Self From(unsigned int       v, unsigned int base = 10);
    static Self From(long               v, unsigned int base = 10);
    static Self From(unsigned long      v, unsigned int base = 10);
    static Self From(long long          v, unsigned int base = 10);
    static Self From(unsigned long long v, unsigned int base = 10);

    template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    T Parse(unsigned int base = 10) const { return AsView().template Parse<T>(base); }

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    T Parse() const { return AsView().template Parse<T>(); }

    View AsView() const;

    const CodeUnit *Data() const;
    std::pair<const CodeUnit*, size_t> DataAndLength() const;

    size_t Length() const;
    bool Empty()    const;

    View Trim()                                     const { return AsView().Trim();                          }
    View TrimLeft()                                 const { return AsView().TrimLeft();                      }
    View TrimRight()                                const { return AsView().TrimRight();                     }
    View Slice(size_t begIdx)                       const { return AsView().Slice(begIdx);                   }
    View Slice(size_t begIdx, size_t endIdx)        const { return AsView().Slice(begIdx, endIdx);           }
    View Prefix(size_t n)                           const { return AsView().Prefix(n);                       }
    View Suffix(size_t n)                           const { return AsView().Suffix(n);                       }
    bool StartsWith(const View &prefix)             const { return AsView().StartsWith(prefix);              }
    bool StartsWith(const Self &prefix)             const { return AsView().StartsWith(prefix);              }
    bool EndsWith(const View &suffix)               const { return AsView().EndsWith(suffix);                }
    bool EndsWith(const Self &suffix)               const { return AsView().EndsWith(suffix);                }
    bool IsDigit(unsigned int base = 10)            const { return AsView().IsDigit(base);                   }
    bool IsDigits(unsigned int base = 10)           const { return AsView().IsDigits(base);                  }
    bool IsAlpha()                                  const { return AsView().IsAlpha();                       }
    bool IsAlphas()                                 const { return AsView().IsAlphas();                      }
    bool IsAlnum(unsigned int base = 10)            const { return AsView().IsAlnum(base);                   }
    bool IsAlnums(unsigned int base = 10)           const { return AsView().IsAlnums(base);                  }
    bool IsUpper()                                  const { return AsView().IsUpper();                       }
    bool IsUppers()                                 const { return AsView().IsUppers();                      }
    bool IsLower()                                  const { return AsView().IsLower();                       }
    bool IsLowers()                                 const { return AsView().IsLowers();                      }
    bool IsWhitespace()                             const { return AsView().IsWhitespace();                  }
    bool IsWhitespaces()                            const { return AsView().IsWhitespaces();                 }
    bool IsASCII()                                  const { return AsView().IsASCII();                       }
    Self ToUpper()                                  const { return AsView().ToUpper();                       }
    Self ToLower()                                  const { return AsView().ToLower();                       }
    Self SwapCase()                                 const { return AsView().SwapCase();                      }
    std::vector<View> Split()                       const { return AsView().Split();                         }
    std::vector<View> Split(const View &spliter)    const { return AsView().Split(spliter);                  }
    std::vector<View> Split(const Self &spliter)    const { return AsView().Split(spliter);                  }
    template<typename C, std::enable_if_t<!std::is_array_v<C>, int> = 0,
                         typename = std::void_t<decltype(std::declval<C>().begin())>>
    std::vector<View> Split(const C &spliters)      const { return AsView().template Split<C>(spliters);     }
    template<typename R> Self Join(R &&strRange)    const { return AsView().Join(std::forward<R>(strRange)); }
    size_t Find(const View &dst, size_t begIdx = 0) const { return AsView().Find(dst, begIdx);               }
    size_t Find(const Self &dst, size_t begIdx = 0) const { return AsView().Find(dst, begIdx);               }

    template<typename F>
    size_t FindCPIf(F &&f) const { return AsView().FindCPIf(std::forward<F>(f)); }

    std::string ToStdString(NativeCharset cs   = NativeCharset::UTF8) const { return AsView().ToStdString(cs); }
    std::wstring ToStdWString(NativeCharset cs = NativeCharset::WUTF) const { return AsView().ToStdWString(cs); }

#if defined(AGZ_OS_WIN32)
    std::wstring ToPlatformString() const { return ToStdWString(); }
#else
    std::string ToPlatformString() const { return ToStdString(); }
#endif

    Iterator begin() const;
    Iterator end()   const;

    CodePointRange<CS> CodePoints() const &  { return CodePointRange<CS>(begin(), end()); }
    CodePointRange<CS> CodePoints() const && { return CodePointRange<CS>(*this, begin(), end()); }

    CharRange Chars() const &  { return CharRange(begin(), end()); }
    CharRange Chars() const && { return CharRange(*this, begin(), end()); }

    std::pair<const CodeUnit*, const CodeUnit*> BeginAndEnd() const { return storage_.BeginAndEnd(); }

    template<typename RHS>
    Self &operator+=(const RHS &rhs) { return *this = *this + rhs; }
};

template<typename CS>
String<CS> operator*(const String<CS> &L, size_t R);
template<typename CS>
String<CS> operator*(size_t L, const String<CS> &R) { return R * L; }
template<typename CS>
String<CS> operator*(const StringView<CS> &L, size_t R) { return L.AsString() * R; }
template<typename CS>
String<CS> operator*(size_t L, const StringView<CS> &R) { return R * L; }

#define AGZ_WRAP_STR_COMP(LHS, RHS, LOP, ROP) \
    template<typename CS> String<CS> operator+(const LHS lhs, const RHS rhs) { return LOP + ROP; } \
    template<typename CS> bool operator==(const LHS lhs, const RHS rhs) { return LOP == ROP; } \
    template<typename CS> bool operator!=(const LHS lhs, const RHS rhs) { return LOP != ROP; } \
    template<typename CS> bool operator< (const LHS lhs, const RHS rhs) { return LOP < ROP; } \
    template<typename CS> bool operator> (const LHS lhs, const RHS rhs) { return LOP > ROP; } \
    template<typename CS> bool operator<=(const LHS lhs, const RHS rhs) { return LOP <= ROP; } \
    template<typename CS> bool operator>=(const LHS lhs, const RHS rhs) { return LOP >= ROP; }

AGZ_WRAP_STR_COMP(String<CS>&,     String<CS>&,     lhs.AsView(),             rhs.AsView())
AGZ_WRAP_STR_COMP(String<CS>&,     StringView<CS>&, lhs.AsView(),             rhs)
AGZ_WRAP_STR_COMP(StringView<CS>&, String<CS>&,     lhs,                      rhs.AsView())
AGZ_WRAP_STR_COMP(String<CS>&,     char*,           lhs,                      String<CS>(rhs))
AGZ_WRAP_STR_COMP(char*,           String<CS>&,     String<CS>(lhs),          rhs)
AGZ_WRAP_STR_COMP(StringView<CS>&, char*,           lhs,                      String<CS>(rhs).AsView())
AGZ_WRAP_STR_COMP(char*,           StringView<CS>&, String<CS>(lhs).AsView(), rhs)
AGZ_WRAP_STR_COMP(String<CS>&,     std::string&,    lhs,                      String<CS>(rhs))
AGZ_WRAP_STR_COMP(std::string&,    String<CS>&,     String<CS>(lhs),          rhs)
AGZ_WRAP_STR_COMP(StringView<CS>&, std::string&,    lhs,                      String<CS>(rhs))
AGZ_WRAP_STR_COMP(std::string&,    StringView<CS>&, String<CS>(lhs),          rhs)
AGZ_WRAP_STR_COMP(String<CS>&,     wchar_t*,        lhs,                      String<CS>(rhs))
AGZ_WRAP_STR_COMP(wchar_t*,        String<CS>&,     String<CS>(lhs),          rhs)
AGZ_WRAP_STR_COMP(StringView<CS>&, wchar_t*,        lhs,                      String<CS>(rhs).AsView())
AGZ_WRAP_STR_COMP(wchar_t*,        StringView<CS>&, String<CS>(lhs).AsView(), rhs)
AGZ_WRAP_STR_COMP(String<CS>&,     std::wstring&,   lhs,                      String<CS>(rhs))
AGZ_WRAP_STR_COMP(std::wstring&,   String<CS>&,     String<CS>(lhs),          rhs)
AGZ_WRAP_STR_COMP(StringView<CS>&, std::wstring&,   lhs,                      String<CS>(rhs))
AGZ_WRAP_STR_COMP(std::wstring&,   StringView<CS>&, String<CS>(lhs),          rhs)

#undef AGZ_WRAP_STR_COMP

template<typename CS>
class StringBuilder
{
    mutable std::list<String<CS>> strs_;

public:

    using Self = StringBuilder<CS>;

    Self &Append(const StringView<CS> &view, size_t n = 1);
    Self &Append(const String<CS> &str, size_t n = 1);

    Self &operator<<(const StringView<CS> &view);

    String<CS> Get() const;

    bool Empty() const { return strs_.empty(); }

    void Clear() { strs_.clear(); }
};

class CharsetConvertor
{
public:

    template<typename DCS, typename SCS>
    static String<DCS> Convert(const typename String<SCS>::View &src);

    template<typename DCS, typename SCS>
    static String<DCS> Convert(const String<SCS> &src) { return Convert<DCS, SCS>(src.AsView()); }
};

AGZ_NS_END(AGZ::StrImpl)

AGZ_NS_BEG(AGZ)

using NativeCharset = StrImpl::NativeCharset;

template<typename CS>
using String = StrImpl::String<CS>;
template<typename CS>
using StringView = StrImpl::StringView<CS>;

using Str8  = String<UTF8<>>;
using Str16 = String<UTF16<>>;
using Str32 = String<UTF32<>>;
using AStr  = String<ASCII<>>;
using WStr  = String<WUTF>;
using PStr  = String<PUTF>;

using StrView8  = StringView<UTF8<>>;
using StrView16 = StringView<UTF16<>>;
using StrView32 = StringView<UTF32<>>;
using AStrView  = StringView<ASCII<>>;
using WStrView  = StringView<WUTF>;
using PStrView  = StringView<PUTF>;

using CSConv = StrImpl::CharsetConvertor;

template<typename CS>
using StringBuilder = StrImpl::StringBuilder<CS>;

template<typename CS>
using CodePointRange = StrImpl::CodePointRange<CS>;

template<typename CS>
using CharRange = typename StrImpl::StringView<CS>::CharRange;

AGZ_NS_END(AGZ)

namespace std
{
    template<typename CS>
    struct hash<AGZ::String<CS>>
    {
        size_t operator()(const AGZ::String<CS>& s) const
        {
            size_t ret = 0;
            for(size_t i = 0; i < s.Length(); i++)
                ret = 65599 * ret + s.Data()[i];
            return ret ^ (ret >> 16);
        }
    };

    template<typename CS>
    struct hash<AGZ::StringView<CS>>
    {
        size_t operator()(const AGZ::StringView<CS>& s) const
        {
            size_t ret = 0;
            for(size_t i = 0; i < s.Length(); i++)
                ret = 65599 * ret + s.Data()[i];
            return ret ^ (ret >> 16);
        }
    };
}
