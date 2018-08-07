#pragma once

#include <atomic>
#include <cstdint>
#include <limits>
#include <list>
#include <string>
#include <vector>

#include "../Misc/Common.h"
#includ "UTF.h"

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
    mutable std::atomic<size_t> refs_;
    E data_[1];

public:

    using Self = RefCountedBuf<E>;

    static Self *New(size_t n);

    RefCountedBuf()              = delete;
    RefCountedBuf(const Self&)   = delete;
    ~RefCountedBuf()             = delete;
    Self &operator=(const Self&) = delete;

    void IncRef() const;
    void DecRef() const;

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
    friend class StringBuilder;

    static constexpr size_t SMALL_BUF_SIZE =
        SmallBufSizeSelector<sizeof(CU)>::Value;

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
            largeBuf_ *buf;
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

    Storage(size_t len);
    Storage(const CU *data, size_t len);
    Storage(const CU *beg, const CU *end);

    Storage(const Self &copyFrom);
    Storage(const Self &copyFrom, size_t begIdx, size_t endIdx);
    Storage(Self &&moveFrom);

    ~Storage();

    Self &operator=(const Self &copyFrom);
    Self &operator=(Self &&moveFrom);

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
class String
{
    friend class StringBuilder;

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
        String<CS> *str_;
        typename CS::CodeUnit *beg_;
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
        size_t Find(const Self &dst, size_t begIdx = 0)   const;
        // Same as Find except it's from right to left
        size_t FindR(const Self &dst, size_t rbegIdx = 0) const;

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

    String(const Self &copyFrom);
    String(Self &&moveFrom);

    ~String() = default;

    operator View() const;

    Self &operator=(const Self &copyFrom);
    Self &operator=(Self &&moveFrom);

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
    Str Join(R &&strRange) const;

    size_t Find(const View &dst, size_t begIdx = 0)   const;
    size_t FindR(const View &dst, size_t rbegIdx = 0) const;

    Iterator begin() const;
    Iterator end()   const;

    std::string ToStdString(NativeCharset cs = NativeCharset::UTF8) const;
};

template<typename CS>
class StringBuilder
{
    std::list<String<CS>> strs_;

public:

    using Self = StringBuilder<CS>;

    Self &Append(const typename String<CS>::View &view);

    Self &operator<<(const typename String<CS>::View &view);

    String<CS> Get() const;

    void Clear();
};

class StringConvertor
{
public:

    template<typename DCS, typename SCS>
    static String<DCS> Convert(const String<SCS>::View &src);
};

AGZ_NS_END(AGZ::StrImpl)

#include "String.inl"
