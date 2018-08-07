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

struct SingleThreaded
{
    using RefCounter = std::size_t;
    static constexpr bool IsThreadSafe = false;
};

struct MultiThreaded
{
    using RefCounter = std::atomic<size_t>;
    static constexpr bool IsThreadSafe = true;
};

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

template<typename E, typename TP>
class RefCountedBuf
{
    mutable typename TP::RefCounter refs_;
    E data_[1];

public:

    using Self = RefCountedBuf<E, TP>;

    static constexpr bool IsThreadSafe = TP::IsThreadSafe;

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

template<typename CU, typename TP>
class Storage
{
    friend class StringBuilder;

    static constexpr size_t SMALL_BUF_SIZE =
        SmallBufSizeSelector<sizeof(CU)>::Value;

    using LargeBuf = RefCountedBuf<CU, TP>;

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

    using Self = Storage<CU, TP>;

    static constexpr bool IsThreadSafe = LargeBuf::IsThreadSafe;

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

template<typename CS, typename TP = MultiThreaded>
class String
{
    friend class StringBuilder;

    Storage<typename CS::CodeUnit, TP> storage_;

    typename CS::CodeUnit *GetMutableData();

    explicit String(size_t len);

public:

    using Charset   = CS;
    using CodeUnit  = typename CS::CodeUnit;
    using CodePoint = typename CS::CodePoint;
    using Self      = String<CS, TP>;

    using Iterator = const CodeUnit*;

    class View
    {
        String<CS, TP> *str_;
        typename CS::CodeUnit *beg_;
        size_t len_;

    public:

        using Charset   = CS;
        using CodeUnit  = typename CS::CodeUnit;
        using CodePoint = typename CS::CodePoint;
        using Self      = View;
        using Str       = String<CS, TP>;

        using Iterator = const CodeUnit*;

        static constexpr size_t NPOS = std::numeric_limits<size_t>::max();

        View(const Str &str);
        View(const Str &str, const CodeUnit* beg, size_t len);
        View(const Str &str, size_t begIdx, size_t endIdx);

        View()                       = delete;
        View(const Self &)           = default;
        ~View()                      = default;
        Self &operator=(const Self &) = default;

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

        bool StartsWith(const Self &prefix) const;
        bool EndsWith(const Self &suffix)   const;

        std::vector<Self> Split()                    const;
        std::vector<Self> Split(const Self &spliter) const;

        template<typename R>
        Str Join(R &&strRange) const;

        size_t Find(const Self &dst, size_t begIdx = 0)   const;
        size_t FindR(const Self &dst, size_t rbegIdx = 0) const;

        Iterator begin() const;
        Iterator end()   const;

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

    template<typename OTP, std::enable_if_t<!std::is_same_v<TP, OTP>, int> = 0>
    String(const String<CS, OTP> &copyFrom);

    ~String() = default;

    Self &operator=(const Self &copyFrom);
    Self &operator=(Self &&moveFrom);

    template<typename OTP, std::enable_if_t<!std::is_same_v<TP, OTP>, int> = 0>
    Self &operator=(const String<CS, OTP> &copyFrom);

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

    bool StartsWith(const Self &prefix) const;
    bool EndsWith(const Self &suffix)   const;

    std::vector<Self> Split()                    const;
    std::vector<Self> Split(const Self &spliter) const;

    template<typename R>
    Str Join(R &&strRange) const;

    size_t Find(const Self &dst, size_t begIdx = 0)   const;
    size_t FindR(const Self &dst, size_t rbegIdx = 0) const;

    Iterator begin() const;
    Iterator end()   const;
};

template<typename CS, typename TP = SingleThreaded>
class StringBuilder
{
    std::list<String<CS, TP>> strs_;

public:

    using Self = StringBuilder<CS>;

    template<typename TPs>
    Self &Append(const typename String<CS, TPs>::View &view);

    template<typename TPs>
    Self &operator<<(const typename String<CS, TPs>::View &view);

    template<typename TPs = MultiThreaded>
    String<CS, TPs> Get() const;

    void Clear();
};

AGZ_NS_END(AGZ::StrImpl)

#include "String.inl"
