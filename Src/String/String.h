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

        operator Str() const;

        // Construct a new string object from this view
        Str AsString() const;

        // Raw data (not zero-terminated)
        const CodeUnit *Data() const;
        // Raw data and count of code units.
        // Usually faster than calling Data() and Length() seperately
        std::pair<const CodeUnit*, size_t> DataAndLength() const;

        // Count of code units
        size_t Length() const;
        // Return true if Length() == 0
        bool Empty()    const;

        // ELiminate whitespaces in both side side
        View Trim()      const;
        // Eliminate whitespaces in left side
        View TrimLeft()  const;
        // Eliminate whitespaces in right side
        View TrimRight() const;

        // Part of this string slice.
        // Same as Suffix(Length() - begIdx)
        View Slice(size_t begIdx)                const;
        // Part of this string slice. Left: inclusive; right: exclusive
        View Slice(size_t begIdx, size_t endIdx) const;

        // Prefix of length n. UB if n > Length().
        View Prefix(size_t n) const;
        // Suffix of length n. UB if n > Length()
        View Suffix(size_t n) const;

        // Is s a prefix of this string?
        bool StartsWith(const Self &s) const;
        // Is s a suffix of this string?
        bool EndsWith(const Self &s)   const;

        // Split with whitespaces
        std::vector<Self> Split()                    const;
        // Split with given string.
        // Example: "xyzabcdefaag" =(split with "a")=> "xyz", "bcdef", "g"
        std::vector<Self> Split(const Self &spliter) const;

        // Concat elements in strRange with this string
        template<typename R>
        Str Join(R &&strRange) const;

        // Search for a substring dst from left to right, starting at begIdx
        // Return NPOS if not found
        size_t Find(const Self &dst, size_t begIdx = 0) const;
        size_t Find(const char *dst, size_t begIdx = 0) const;

        // Begin iterator for traversal code units
        Iterator begin() const;
        // End iterator for traversal code units
        Iterator end()   const;

        // Convert self to std::string with specified encoding (default to UTF8)
        std::string ToStdString(NativeCharset cs = NativeCharset::UTF8) const;

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

    operator View() const;

    Self &operator=(const Self &copyFrom);
    Self &operator=(Self &&moveFrom) noexcept;

    // Construct a view for the whole string
    View AsView() const;

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

    bool StartsWith(const View &prefix) const;
    bool EndsWith(const View &suffix)   const;

    std::vector<View> Split()                    const;
    std::vector<View> Split(const View &spliter) const;

    template<typename R>
    Self Join(R &&strRange) const;

    size_t Find(const View &dst, size_t begIdx = 0) const;
    size_t Find(const char *dst, size_t begIdx = 0) const;

    Iterator begin() const;
    Iterator end()   const;

    std::string ToStdString(NativeCharset cs = NativeCharset::UTF8) const;

    bool operator==(const Self &rhs) const;
    bool operator!=(const Self &rhs) const;
    bool operator< (const Self &rhs) const;
    bool operator> (const Self &rhs) const;
    bool operator<=(const Self &rhs) const;
    bool operator>=(const Self &rhs) const;
};

#define AGZ_WRAP_STR_COMP(LHS, RHS, LOP, ROP) \
    template<typename CS> \
    bool operator==(const LHS lhs, const RHS rhs) {  return LOP == ROP; } \
    template<typename CS> \
    bool operator!=(const LHS lhs, const RHS rhs) { return LOP != ROP; } \
    template<typename CS> \
    bool operator<(const LHS lhs, const RHS rhs) { return LOP < ROP; } \
    template<typename CS> \
    bool operator>(const LHS lhs, const RHS rhs) { return LOP > ROP; } \
    template<typename CS> \
    bool operator<=(const LHS lhs, const RHS rhs) { return LOP <= ROP; } \
    template<typename CS> \
    bool operator>=(const LHS lhs, const RHS rhs) { return LOP >= ROP; }

AGZ_WRAP_STR_COMP(String<CS>&, typename String<CS>::View&, lhs.AsView(), rhs)
AGZ_WRAP_STR_COMP(typename String<CS>::View&, String<CS>&, lhs, rhs.AsView())
AGZ_WRAP_STR_COMP(String<CS>&, char*, lhs, String<CS>(rhs))
AGZ_WRAP_STR_COMP(char*, String<CS>&, String<CS>(lhs), rhs)
AGZ_WRAP_STR_COMP(typename String<CS>::View&, char*, lhs, String<CS>(rhs).AsView())
AGZ_WRAP_STR_COMP(char*, typename String<CS>::View&, String<CS>(lhs).AsView(), rhs)
AGZ_WRAP_STR_COMP(String<CS>&, std::string&, lhs, String<CS>(rhs))
AGZ_WRAP_STR_COMP(std::string&, String<CS>&, String<CS>(lhs), rhs)
AGZ_WRAP_STR_COMP(typename String<CS>::View&, std::string&, lhs, String<CS>(rhs))
AGZ_WRAP_STR_COMP(std::string&, typename String<CS>::View&, String<CS>(lhs), rhs)

#undef AGZ_WRAP_STR_COMP

template<typename CS>
class StringBuilder
{
    std::list<String<CS>> strs_;

public:

    using Self = StringBuilder<CS>;

    Self &Append(const typename String<CS>::View &view);
    Self &Append(const String<CS> &str);

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
    static String<DCS> Convert(const String<SCS> &src);
};

AGZ_NS_END(AGZ::StrImpl)

AGZ_NS_BEG(AGZ)

using Str8  = StrImpl::String<UTF8<>>;
using Str16 = StrImpl::String<UTF16<>>;
using Str32 = StrImpl::String<UTF32<>>;
using AStr  = StrImpl::String<ASCII<>>;
using WStr  = StrImpl::String<WUTF>;

using CSConv = StrImpl::CharsetConvertor;

AGZ_NS_END(AGZ)

#include "String.inl"
