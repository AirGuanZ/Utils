#pragma once

#include <atomic>
#include <ostream>
#include <string>

#include "../Misc/Common.h"
#include "../Range/Reverse.h"
#include "Algorithm.h"
#include "UTF.h"

AGZ_NS_BEG(AGZ)

template<typename CS, typename TP> class String;

// Possible encoding used by a c-style string
enum class CharEncoding { UTF8 };

namespace StringAux
{
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

    // Reference counted buffer for large string storage
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

        static Self *New(size_t n);
    };

    static constexpr size_t SMALL_BUF_SIZE = 31;
}

template<typename CS, typename TP>
class CharRange
{
    static constexpr size_t SMALL_BUF_SIZE = StringAux::SMALL_BUF_SIZE;

    using LargeBuf = StringAux::RefCountedBuf<typename CS::CodeUnit, TP>;

    union
    {
        typename CS::CodeUnit smallBuf_[SMALL_BUF_SIZE];
        LargeBuf *largeBuf_;
    };

    bool small_;

    const typename CS::CodeUnit *beg_;
    const typename CS::CodeUnit *end_;

public:

    using Iterator        = typename CS::Iterator;
    using ReverseIterator = ::AGZ::ReverseIterator<Iterator>;
    using CodeUnit        = typename CS::CodeUnit;

    using Self = CharRange<CS, TP>;

    // For large storage
    CharRange(LargeBuf *buf, const CodeUnit *beg, const CodeUnit *end);
    // For small storage
    CharRange(const CodeUnit *beg, const CodeUnit *end);

    CharRange(const Self &copyFrom);

    Self &operator=(const Self &) = delete;

    ~CharRange() { if(!small_) largeBuf_->DecRef(); }

    Iterator begin() const { return Iterator(beg_); }
    Iterator end() const { return Iterator(end_); }

    ReverseIterator rbegin() const { return ReverseIterator(end()); }
    ReverseIterator rend() const { return ReverseIterator(begin()); }
};

// CU: Code Unit
// CP: Code Point
// TP: Thread Policy
template<typename CS = UTF8<char>, typename TP = StringAux::MultiThreaded>
class String
{
    static constexpr size_t SMALL_BUF_SIZE = StringAux::SMALL_BUF_SIZE;

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

    const typename CS::CodeUnit *End() const;

    void Init(const typename CS::CodeUnit *beg,
              const typename CS::CodeUnit *end);
    void Init2(const typename CS::CodeUnit *beg1,
               const typename CS::CodeUnit *end1,
               const typename CS::CodeUnit *beg2,
               const typename CS::CodeUnit *end2);

    String<CS, TP> &CopyFromSelf(const String<CS, TP> &copyFrom);
    void ConstructFromSelf(const String<CS, TP> &copyFrom);

public:

    using CharSet   = CS;
    using CodeUnit  = typename CS::CodeUnit;
    using CodePoint = typename CS::CodePoint;
    using Self      = String<CS, TP>;

    using Iterator        = const CodeUnit*;
    using ReverseIterator = ReverseIterator<Iterator>;

    String();

    // Construct from existed buffer
    // Will copy all data to owned storage
    // Faster than the checking version bellow. UB when [beg, end) is invalid
    String(CONS_FLAG_NOCHECK_t, const CodeUnit *beg, const CodeUnit *end);

    // Construct from existed buffer
    // Will copy all data to owned storage
    String(const CodeUnit *beg, const CodeUnit *end);

    // Construct with concat([beg1, end1), [beg2, end2))
    String(CONS_FLAG_NOCHECK_t, const CodeUnit *beg1, const CodeUnit *end1,
                                const CodeUnit *beg2, const CodeUnit *end2);

    // Construct with concat([beg1, end1), [beg2, end2))
    String(const CodeUnit *beg1, const CodeUnit *end1,
           const CodeUnit *beg2, const CodeUnit *end2);

    // Construct from existed buffer
    // Will copy all data to owned storage
    String(const CodeUnit *beg, size_t n) : String(beg, beg + n) { }

    // Construct from existed buffer using another CharSet
    template<typename OCS>
    String(CONS_FLAG_FROM_t<OCS>, const typename OCS::CodeUnit *beg,
                                  const typename OCS::CodeUnit *end);

    // Construct from existed buffer using another CharSet
    template<typename OCS>
    String(CONS_FLAG_FROM_t<OCS>, const typename OCS::CodeUnit *beg,
                                  size_t n);

    template<typename OCS, typename OTP>
    explicit String(const String<OCS, OTP> &copyFrom);

    String(const Self &copyFrom);

    String(Self &&moveFrom) noexcept;

    String(const CodeUnit *beg, const CodeUnit *end, size_t repeat);

    String(const char *cStr);
    String(const std::string &cppStr);
    String(const char *cStr, CharEncoding encoding);
    String(const std::string &cppStr, CharEncoding encoding);

    template<typename OCS, typename OTP>
    Self &operator=(const String<OCS, OTP> &copyFrom);

    Self &operator=(const Self &copyFrom);

    Self &operator=(Self &&moveFrom) noexcept;

    ~String();

    void Swap(Self &other);

    // Set with a new value and return the old one
    Self Exchange(const Self &value);

    const CodeUnit *Data() const;

    size_t Length() const;

    bool IsEmpty() const;

    std::pair<const CodeUnit*, size_t> DataAndLength() const;
    std::pair<const CodeUnit*, const CodeUnit*> BeginAndEnd() const;

    std::string ToStdString() const;

    CodeUnit operator[](size_t idx) const;
    Self operator+(const Self &rhs);
    Self operator*(size_t n);

    CharRange<CS, TP> Chars() const;

    Iterator begin() const;
    Iterator end() const;

    ReverseIterator rbegin() const;
    ReverseIterator rend() const;

    bool StartsWith(const Self &prefix) const;
    bool EndsWith(const Self &suffix) const;

    constexpr static size_t NPOS = StrAlgo::NPOS;

    size_t Find(const Self &dst) const;
    size_t RFind(const Self &dst) const;

    bool operator==(const Self &rhs) const;
    bool operator!=(const Self &rhs) const;
    bool operator<(const Self &rhs) const;
    bool operator<=(const Self &rhs) const;
    bool operator>=(const Self &rhs) const;
    bool operator>(const Self &rhs) const;

    bool operator==(const std::string &rhs) const;
    bool operator!=(const std::string &rhs) const;
    bool operator<(const std::string &rhs) const;
    bool operator<=(const std::string &rhs) const;
    bool operator>=(const std::string &rhs) const;
    bool operator>(const std::string &rhs) const;

    bool operator==(const char *rhs) const;
    bool operator!=(const char *rhs) const;
    bool operator<(const char *rhs) const;
    bool operator<=(const char *rhs) const;
    bool operator>=(const char *rhs) const;
    bool operator>(const char *rhs) const;
};

template<typename CS, typename TP, typename R>
String<CS, TP> &operator+=(String<CS, TP> &lhs, R &&rhs);

template<typename CS, typename TP>
bool operator==(const std::string &lhs, const String<CS, TP> &rhs);
template<typename CS, typename TP>
bool operator!=(const std::string &lhs, const String<CS, TP> &rhs);
template<typename CS, typename TP>
bool operator<(const std::string &lhs, const String<CS, TP> &rhs);
template<typename CS, typename TP>
bool operator<=(const std::string &lhs, const String<CS, TP> &rhs);
template<typename CS, typename TP>
bool operator>=(const std::string &lhs, const String<CS, TP> &rhs);
template<typename CS, typename TP>
bool operator>(const std::string &lhs, const String<CS, TP> &rhs);

template<typename CS, typename TP>
bool operator==(const char *lhs, const String<CS, TP> &rhs);
template<typename CS, typename TP>
bool operator!=(const char *lhs, const String<CS, TP> &rhs);
template<typename CS, typename TP>
bool operator<(const char *lhs, const String<CS, TP> &rhs);
template<typename CS, typename TP>
bool operator<=(const char *lhs, const String<CS, TP> &rhs);
template<typename CS, typename TP>
bool operator>=(const char *lhs, const String<CS, TP> &rhs);
template<typename CS, typename TP>
bool operator>(const char *lhs, const String<CS, TP> &rhs);

template<typename CS, typename TP>
String<CS, TP> operator*(size_t n, const String<CS, TP> &s);

template<typename CS, typename TP>
std::ostream &operator<<(std::ostream &out, const String<CS, TP> &s);

using Str8  = String<UTF8<>>;
using Str16 = String<UTF16<>>;
using Str32 = String<UTF32<>>;
using WStr  = String<WUTF>;

struct StringJoinRHS { Str8 mid, empty; };

inline StringJoinRHS Join(const Str8 &mid = Str8(" "),
                          const Str8 &empty = Str8(""));

template<typename R>
auto operator|(const R &strs, StringJoinRHS rhs);

AGZ_NS_END(AGZ)

#include "String.inl"
