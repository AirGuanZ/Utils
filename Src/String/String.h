#pragma once

#include <atomic>
#include <cstdint>
#include <limits>
#include <list>
#include <string>
#include <vector>

#include "../Misc/Common.h"
#include "ASCII.h"
#include "UTF.h"

AGZ_NS_BEG(AGZ::StrImpl)

// Charsets that can be used by c-style string and std::string
// Code unit must be alias of char
enum class NativeCharset
{
    UTF8
};

// Determinating how many values can SSO buffer hold.
template<size_t>
struct SmallBufSizeSelector;

template<>
struct SmallBufSizeSelector<1>
{
    static constexpr size_t Value = 31;
};

template<>
struct SmallBufSizeSelector<2>
{
    static constexpr size_t Value = 15;
};

template<>
struct SmallBufSizeSelector<4>
{
    static constexpr size_t Value = 7;
};

// Reference counted value container
// Used for large string storage
template<typename E>
class RefCountedBuf
{
    std::atomic<size_t> refs_;
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
    static constexpr size_t SMALL_BUF_SIZE =
        SmallBufSizeSelector<sizeof(CU)>::Value;

    using LargeBuf = RefCountedBuf<CU>;

    template<typename CS>
    friend class String;

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

    CU *GetMutableData();

public:

    using Self = Storage<CU>;

    static_assert(std::is_trivially_copyable_v<CU>);

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

template<typename CS>
class StringBuilder;

template<typename CS>
class String
{
    friend class StringBuilder<CS>;

    Storage<typename CS::CodeUnit> storage_;

    typename CS::CodeUnit *GetMutableData();

    explicit String(size_t len);

public:

    using Charset   = CS;
    using CodeUnit  = typename CS::CodeUnit;
    using CodePoint = typename CS::CodePoint;
    using Self      = String<CS>;

    using Iterator = const CodeUnit*;

    // Immutable string slice
    class View
    {
        const String<CS> *str_;
        const typename CS::CodeUnit *beg_;
        size_t len_;

    public:

        using Charset   = CS;
        using CodeUnit  = typename CS::CodeUnit;
        using CodePoint = typename CS::CodePoint;
        using Self      = View;
        using Str       = String<CS>;

        using Iterator = const CodeUnit*;

        static constexpr size_t NPOS = std::numeric_limits<size_t>::max();

        View(const Str &str);
        View(const Str &str, const CodeUnit* beg, size_t len);
        View(const Str &str, size_t begIdx, size_t endIdx);

        View()                        = delete;
        View(const Self &)            = default;
        ~View()                       = default;
        Self &operator=(const Self &) = default;

        Str AsString() const;

        const CodeUnit *Data() const;
        std::pair<const CodeUnit*, size_t> DataAndLength() const;

        size_t Length() const;
        bool Empty()    const;

        View Trim()      const;
        View TrimLeft()  const;
        View TrimRight() const;

        View Slice(size_t begIdx)                const;
        View Slice(size_t begIdx, size_t endIdx) const;

        View Prefix(size_t n) const;
        View Suffix(size_t n) const;

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

        Str ToUpper() const;
        Str ToLower() const;
        Str SwapCase() const;

        std::vector<Self> Split()                    const;
        std::vector<Self> Split(const Self &spliter) const;
        std::vector<Self> Split(const Str &spliter)  const { return Split(spliter.AsView()); }

        template<typename R>
        Str Join(R &&strRange) const;

        size_t Find(const Self &dst, size_t begIdx = 0) const;
        size_t Find(const Str &dst, size_t begIdx = 0)  const { return Find(dst.AsView(), begIdx); }

        std::string ToStdString(NativeCharset cs = NativeCharset::UTF8) const;

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

    static constexpr size_t NPOS = View::NPOS;

    String();
    String(const View &view);
    String(const CodeUnit *beg, size_t len);
    String(const CodeUnit *beg, const CodeUnit *end);
    String(const Self &copyFrom, size_t begIdx, size_t endIdx);

    String(const char *cstr, NativeCharset cs = NativeCharset::UTF8);
    String(const std::string &cppStr, NativeCharset cs = NativeCharset::UTF8);

    String(const Self &copyFrom);
    String(Self &&moveFrom) noexcept;

    ~String() = default;

    Self &operator=(const Self &copyFrom);
    Self &operator=(Self &&moveFrom) noexcept;

    static Self From(char v,               unsigned int base = 10);
    static Self From(signed char v,        unsigned int base = 10);
    static Self From(unsigned char v,      unsigned int base = 10);
    static Self From(short v,              unsigned int base = 10);
    static Self From(unsigned short v,     unsigned int base = 10);
    static Self From(int v,                unsigned int base = 10);
    static Self From(unsigned int v,       unsigned int base = 10);
    static Self From(long v,               unsigned int base = 10);
    static Self From(unsigned long v,      unsigned int base = 10);
    static Self From(long long v,          unsigned int base = 10);
    static Self From(unsigned long long v, unsigned int base = 10);

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
    Self ToUpper()                                  const { return AsView().ToUpper();                       }
    Self ToLower()                                  const { return AsView().ToLower();                       }
    Self SwapCase()                                 const { return AsView().SwapCase();                      }
    std::vector<View> Split()                       const { return AsView().Split();                         }
    std::vector<View> Split(const View &spliter)    const { return AsView().Split(spliter);                  }
    std::vector<View> Split(const Self &spliter)    const { return AsView().Split(spliter);                  }
    template<typename R> Self Join(R &&strRange)    const { return AsView().Join(std::forward<R>(strRange)); }
    size_t Find(const View &dst, size_t begIdx = 0) const { return AsView().Find(dst, begIdx);               }
    size_t Find(const Self &dst, size_t begIdx = 0) const { return AsView().Find(dst, begIdx);               }
    std::string ToStdString(NativeCharset cs = NativeCharset::UTF8) const { return AsView().ToStdString(cs); }

    Iterator begin() const;
    Iterator end()   const;
};

template<typename CS>
String<CS> operator*(const String<CS> &L, size_t R);
template<typename CS>
String<CS> operator*(size_t L, const String<CS> &R) { return R * L; }
template<typename CS>
String<CS> operator*(const typename String<CS>::View &L, size_t R) { return L.AsString() * R }
template<typename CS>
String<CS> operator*(size_t L, const typename String<CS>::View &R) { return R * L; }

#define AGZ_WRAP_STR_COMP(LHS, RHS, LOP, ROP) \
    tempate<typename CS> String<CS> operator+(const LHS lhs, const RHS rhs) { return LOP + ROP; } \
    template<typename CS> bool operator==(const LHS lhs, const RHS rhs) { return LOP == ROP; } \
    template<typename CS> bool operator!=(const LHS lhs, const RHS rhs) { return LOP != ROP; } \
    template<typename CS> bool operator< (const LHS lhs, const RHS rhs) { return LOP < ROP; } \
    template<typename CS> bool operator> (const LHS lhs, const RHS rhs) { return LOP > ROP; } \
    template<typename CS> bool operator<=(const LHS lhs, const RHS rhs) { return LOP <= ROP; } \
    template<typename CS> bool operator>=(const LHS lhs, const RHS rhs) { return LOP >= ROP; }

AGZ_WRAP_STR_COMP(String<CS>&,                String<CS>&,                lhs.AsView(),             rhs.AsView())
AGZ_WRAP_STR_COMP(String<CS>&,                typename String<CS>::View&, lhs.AsView(),             rhs)
AGZ_WRAP_STR_COMP(typename String<CS>::View&, String<CS>&,                lhs,                      rhs.AsView())
AGZ_WRAP_STR_COMP(String<CS>&,                char*,                      lhs,                      String<CS>(rhs))
AGZ_WRAP_STR_COMP(char*,                      String<CS>&,                String<CS>(lhs),          rhs)
AGZ_WRAP_STR_COMP(typename String<CS>::View&, char*,                      lhs,                      String<CS>(rhs).AsView())
AGZ_WRAP_STR_COMP(char*,                      typename String<CS>::View&, String<CS>(lhs).AsView(), rhs)
AGZ_WRAP_STR_COMP(String<CS>&,                std::string&,               lhs,                      String<CS>(rhs))
AGZ_WRAP_STR_COMP(std::string&,               String<CS>&,                String<CS>(lhs),          rhs)
AGZ_WRAP_STR_COMP(typename String<CS>::View&, std::string&,               lhs,                      String<CS>(rhs))
AGZ_WRAP_STR_COMP(std::string&,               typename String<CS>::View&, String<CS>(lhs),          rhs)

#undef AGZ_WRAP_STR_COMP

template<typename CS>
class StringBuilder
{
    mutable std::list<String<CS>> strs_;

public:

    using Self = StringBuilder<CS>;

    Self &Append(const typename String<CS>::View &view, size_t n = 1);
    Self &Append(const String<CS> &str, size_t n = 1);

    Self &operator<<(const typename String<CS>::View &view);

    String<CS> Get() const;

    void Clear();
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

template<typename CS>
using String = StrImpl::String<CS>;

using Str8  = String<UTF8<>>;
using Str16 = String<UTF16<>>;
using Str32 = String<UTF32<>>;
using AStr  = String<ASCII<>>;
using WStr  = String<WUTF>;

using CSConv  = StrImpl::CharsetConvertor;

template<typename CS>
using StringBuilder = StrImpl::StringBuilder<CS>;

AGZ_NS_END(AGZ)

#include "String.inl"
