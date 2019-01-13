#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <limits>
#include <list>
#include <string>
#include <vector>

#include "../../Misc/Common.h"
#include "../../Range/Iterator.h"
#include "../../Utils/Serialize.h"
#include "../Charset/ASCII.h"
#include "../Charset/UTF.h"

namespace AGZ::StrImpl {

/**
 * @brief C style string和std::string可能使用的编码，作为编码转换的参数
 */
enum class NativeCharset
{
    UTF8, ///< for const char * / std::string
    WUTF, ///< for const wchar_t * / std::wstring
};

/**
 * @brief 使用引用计数的缓存块
 */
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

    /** 创建一块含有n个可用字节的引用计数缓存 */
    static RefCountedBuf<E> *New(size_t n);

    /** 不允许默认构造，只能通过 RefCountedBuf<E>::New 来创建 */
    RefCountedBuf()                                      = delete;
    /** 不允许复制，只能通过指针共享 */
    RefCountedBuf(const RefCountedBuf<E>&)               = delete;
    /** 不允许用户析构，只能使用引用计数自动管理 */
    ~RefCountedBuf()                                     = delete;
    /** 不允许赋值，只能通过指针共享 */
    RefCountedBuf<E> &operator=(const RefCountedBuf<E>&) = delete;

    /** 将引用计数增加1 */
    void IncRef();
    /** 将引用计数减少1，如减少后计数为0，则释放缓存及元数据 */
    void DecRef();

    /** 取得引用计数数量 */
    size_t GetRefCount() const { return refs_; }

    /** 取得缓存区域指针 */
    E *GetData();
    /** 取得缓存区域指针 */
    const E *GetData() const;
};

/**
 * @brief 带SSO的字符串存储
 * 
 * 小字符串被存储在栈上，大字符串则用引用计数共享，除了内部实现外，对外不可变
 */
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

    bool IsSmallStorage() const;
    bool IsLargeStorage() const;

    size_t GetSmallLength() const;
    size_t GetLargeLength() const;

public:

    /** 自身类型 */
    using Self = Storage<CU>;

    static_assert(std::is_trivially_copyable_v<CU>);

    /**
     * @brief 以可变方式取得内部首元素地址
     * 
     * 前置条件：IsSmallStorage或RefCount = 1
     */
    CU *GetMutableData();

    //! 准备长度为len的缓存，但不初始化其内容
    explicit Storage(size_t len);
    //! 准备长度为len的缓存，并用data初始化它
    Storage(const CU *data, size_t len);
    //! 准备合适长度的缓存，保存[beg, end)间的内容
    Storage(const CU *beg, const CU *end);

    /**
     * 复制copyFrom的内容。若copyFrom长度较小，则逐字节拷贝数据；否则共享引用计数
     */
    Storage(const Self &copyFrom);
    /**
     * 复制copyFrom中下标范围为[begIdx, endIdx)的码元子串
     * 若子串长度较小，则逐字节拷贝数据；否则共享引用计数
     */
    Storage(const Self &copyFrom, size_t begIdx, size_t endIdx);
    /** 取得其他Storage对象中的数据 */
    Storage(Self &&moveFrom) noexcept;

    /** 销毁时，若内部持有子串的方式是引用计数，则令计数减1 */
    ~Storage();

    Storage<CU> &operator=(const Self &copyFrom);
    Storage<CU> &operator=(Self &&moveFrom) noexcept;

    //! 取得缓存大小
    size_t GetLength() const;

    //! 取得首元素指针
    const CU *Begin() const;
    //! 取得最后一个元素的下一个元素的指针
    const CU *End() const;

    //! 同时取得首元素地址和内部存储的串长度
    std::pair<const CU*, size_t> BeginAndLength() const;
    //! 同时取得首元素地址和末元素下一个元素的地址
    std::pair<const CU*, const CU*> BeginAndEnd() const;
};

/**
 * @brief 不带SSO优化的字符串存储，接口含义和 Storage<CU> 相同
 */
template<typename CU>
class Storage_NoSSO
{
    using Buf = RefCountedBuf<CU>;
    Buf *buf_;
    CU *beg_, *end_;

    void Alloc(size_t len);

public:

    /** 自身类型 */
    using Self = Storage_NoSSO<CU>;

    static_assert(std::is_trivially_copyable_v<CU>);

    //! 以可变方式取得内部数据
    CU *GetMutableData();

    explicit Storage_NoSSO(size_t len);
    Storage_NoSSO(const CU *data, size_t len);
    Storage_NoSSO(const CU *beg, const CU *end);

    Storage_NoSSO(const Self &copyFrom);
    Storage_NoSSO(const Self &copyFrom, size_t begIdx, size_t endIdx);
    Storage_NoSSO(Self &&moveFrom) noexcept;

    ~Storage_NoSSO();

    Storage_NoSSO<CU> &operator=(const Self &copyFrom);
    Storage_NoSSO<CU> &operator=(Self &&moveFrom) noexcept;

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

/**
 * @brief 用于遍历字符串中的码点的range对象
 */
template<typename CS>
class CodePointRange
{
    String<CS> str_;
    const typename CS::CodeUnit *beg_;
    const typename CS::CodeUnit *end_;

public:

    using CodeUnit  = typename CS::CodeUnit;  ///< 码元类型
    using CodePoint = typename CS::CodePoint; ///< 码点类型
    using Iterator  = GetIteratorType<CS>;    ///< 码元迭代器类型

    //! 用beg和end初始化该range，此时range中仅保存首尾指针
    CodePointRange(const CodeUnit *beg, const CodeUnit *end);

    //! range中会保存整个字符串的内容
    CodePointRange(const String<CS> &str, const CodeUnit *beg,
                                          const CodeUnit *end);

    //! 取得头部码元迭代器
    Iterator begin() const;
    //! 取得尾部码元迭代器
    Iterator end()   const;

    //! 给定码点迭代器，求它的第一个码元在该range的码元中的下标
    size_t CodeUnitIndex(const Iterator &it) const
    {
        return CS::CodeUnitsBeginFromCodePointIterator(it) - beg_;
    }
};

/**
* @brief 以子串的形式遍历整个字符串中单个字符（码点）的range对象
*/
template<typename CS>
class CharRange
{
    using InIt = GetIteratorType<CS>;

    CodePointRange<CS> CPR_;

public:

    using CodeUnit = typename CS::CodeUnit;   ///< 码元类型
    using CodePoint = typename CS::CodePoint; ///< 码点类型

    /**
     * 迭代器类型
     */
    class Iterator
    {
        InIt it_;

    public:

        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = String<CS>;
        using difference_type =
            typename std::iterator_traits<InIt>::difference_type;
        using pointer = ValuePointer<value_type>;
        using reference = value_type & ;

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
    CharRange(const String<CS> &str, const CodeUnit *beg, const CodeUnit *end);

    Iterator begin() const;
    Iterator end()   const;
};

/**
 * @brief 不可变字符串视图
 */
template<typename CS>
class StringView
{
    const String<CS> *str_;
    const typename CS::CodeUnit *beg_;
    size_t len_;

public:

    using Charset = CS;                       ///< 字符编码方案
    using CodeUnit = typename CS::CodeUnit;   ///< 码元类型
    using CodePoint = typename CS::CodePoint; ///< 码点类型
    using Self = StringView<CS>;              ///< 自身类型
    using Str = String<CS>;                   ///< 对应的字符串类型

    using Iterator = const CodeUnit*; ///! 用于遍历码元的迭代器类型

    //! 用于标记查找失败等特殊情形的下标，语义上表示+infinity
    static constexpr size_t NPOS = std::numeric_limits<size_t>::max();

    //! 取某个字符串的视图
    StringView(const Str &str);
    //! 取某字符串的子串视图，并将该串缓存到视图中
    StringView(const Str &str, const CodeUnit *beg, size_t len);
    //! 取某字符串的子串视图，并将该串缓存到视图中
    StringView(const Str &str, size_t begIdx, size_t endIdx);

    /** 不允许自行构造，只能从 String<CS> 获得 */
    StringView()                  = delete;
    StringView(const Self &)      = default;
    ~StringView()                 = default;
    StringView<CS> &operator=(const Self &) = default;

    //! 转换为对应的String
    Str AsString() const;

    //! 取得内部保存的码元数据
    const CodeUnit *Data() const;
    //! 取得内部保存的码元数据以及长度
    std::pair<const CodeUnit*, size_t> DataAndLength() const;

    //! 码元数量
    size_t Length() const;
    //! 是否为空串
    bool Empty()    const;

    //! 以给定进制转换为指定类型的整数
    template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    T Parse(unsigned base = 10) const;

    //! 转换为指定类型的浮点数
    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    T Parse() const;

    //! 返回消除左侧空白字符后的字符串
    Self Trim()      const;
    //! 返回消除右侧空白字符后的字符串
    Self TrimLeft()  const;
    //! 返回消除两侧空白字符后的字符串
    Self TrimRight() const;

    //! 返回从以begIdx为下标的码元开始，直到结尾的子串
    Self Slice(size_t begIdx)                const;
    //! 返回下标范围位于[begIdx, endIdx)间的码元构成的子串
    Self Slice(size_t begIdx, size_t endIdx) const;

    //! 返回前n个码元构成的子串
    Self Prefix(size_t n) const;
    //! 返回后n个码元构成的子串
    Self Suffix(size_t n) const;

    //! 是否以给定的字符串开头
    bool StartsWith(const Self &s) const;
    //! 是否以给定的字符串开头
    bool StartsWith(const Str &s)  const { return StartsWith(s.AsView()); }

    //! 是否以给定的字符串结尾
    bool EndsWith(const Self &s) const;
    //! 是否以给定的字符串结尾
    bool EndsWith(const Str &s)  const { return EndsWith(s.AsView()); }

    //! 是否是单个数字字符
    bool IsDigit(unsigned int base = 10)  const;
    //! 是否是数字串
    bool IsDigits(unsigned int base = 10) const;

    //! 是否是单个英文字母
    bool IsAlpha()  const;
    //! 是否是英文字母串
    bool IsAlphas() const;

    //! 是否是单个数字/英文字母
    bool IsAlnum(unsigned int base = 10)  const;
    //! 是否是数字/英文字母串
    bool IsAlnums(unsigned int base = 10) const;

    //! 是否是单个大写英文字母
    bool IsUpper()  const;
    //! 是否是大写字母串
    bool IsUppers() const;

    //! 是否是单个小写字母
    bool IsLower()  const;
    //! 是否是小写字母串
    bool IsLowers() const;

    //! 是否是单个空白字符
    bool IsWhitespace()  const;
    //! 是否是空白字符串
    bool IsWhitespaces() const;

    //! 是否是ASCII字符串
    bool IsASCII() const;

    //! 将小写英文字母转换为大写，其余字符不变
    Str ToUpper()  const;
    //! 将大写英文字母转换为小写，其余字符不变
    Str ToLower()  const;
    //! 将英文字母大小写互换，其余字符不变
    Str SwapCase() const;

    //! 以空白字符为分隔符拆分此串
    std::vector<Self> Split()                    const;
    //! 以给定的字符串为分隔符拆分此串
    std::vector<Self> Split(const Self &spliter) const;
    //! 以给定的字符串为分隔符拆分此串
    std::vector<Self> Split(const Str &spliter)  const { return Split(spliter.AsView()); }

    //! 以给定的字符串集合中的任意一个为分隔符拆分此串
    template<typename C, std::enable_if_t<!std::is_array_v<C>, int> = 0,
                         typename = std::void_t<decltype(std::declval<C>().begin())>>
    std::vector<Self> Split(const C &spliters) const;

    //! 以自己为分隔符连接一个字符串range中的所有元素
    template<typename R>
    Str Join(R &&strRange) const;

    /**
     * @brief 从以begIdx为下标的码元开始查找子串
     * 
     * @return 查找失败时返回NPOS
     */
    size_t Find(const Self &dst, size_t begIdx = 0) const;
    //! @copydoc StringView<CS>::Find(const StringView<CS>&, size_t) const
    size_t Find(const Str &dst, size_t begIdx = 0)  const { return Find(dst.AsView(), begIdx); }

    /**
     * @brief 查找第一个满足给定谓词的码点的第一个码元的下标
     * 
     * @return 查找失败时返回NPOS
     */
    template<typename F>
    size_t FindCPIf(F &&f) const;

    //! 用于遍历码点的range对象
    CodePointRange<CS> CodePoints() const &  { return CodePointRange<CS>(beg_, beg_ + len_); }
    //! 用于遍历码点的range对象
    CodePointRange<CS> CodePoints() const && { return CodePointRange<CS>(*str_, beg_, beg_ + len_); }

    //! 用于以子串形式遍历字符串中每个字符的range对象
    CharRange<CS> Chars() const &  { return CharRange(beg_, beg_ + len_); }
    //! 用于以子串形式遍历字符串中每个字符的range对象
    CharRange<CS> Chars() const && { return CharRange(*str_, beg_, beg_ + len_); }

    //! 转换为指定编码的std::string，默认使用UTF-8
    std::string ToStdString(NativeCharset cs = NativeCharset::UTF8)  const;
    //! 转换为宽字符串，默认编码和平台有关
    std::wstring ToStdWString(NativeCharset cs = NativeCharset::WUTF) const;

#if defined(AGZ_OS_WIN32)
    //! 转换为平台默认使用的字符串
    std::wstring ToPlatformString() const { return ToStdWString(); }
#else
    //! 转换为平台默认使用的字符串
    std::string ToPlatformString() const { return ToStdString(); }
#endif

    //! 用于码元遍历的头部迭代器
    Iterator begin() const;
    //! 用于码元遍历的尾部迭代器
    Iterator end()   const;

    //! 和其他字符串连接产生新串
    Str operator+(const Self &rhs) const;

    /** 字典序等于 */
    bool operator==(const Self &rhs) const;
    /** 字典序不等于 */
    bool operator!=(const Self &rhs) const;
    /** 字典序小于 */
    bool operator< (const Self &rhs) const;
    /** 字段序大于 */
    bool operator> (const Self &rhs) const;
    /** 字典序小于等于 */
    bool operator<=(const Self &rhs) const;
    /** 字段序大于等于 */
    bool operator>=(const Self &rhs) const;
};

/**
 * @brief 不可变的字符串类
 */
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

    using Charset   = CS;                      ///< 字符编码方案
    using CodeUnit  = typename CS::CodeUnit;   ///< 码元类型
    using CodePoint = typename CS::CodePoint;  ///< 码点类型
    using Builder   = StringBuilder<CS>;       ///< 字符串构造器
    using View      = StringView<CS>;          ///< 视图类型
    using Self      = String<CS>;              ///< 自身类型

    using Iterator = const CodeUnit*; ///< 遍历码元的迭代器类型

    //! 用于标记查找失败等特殊情形的下标，语义上表示+infinity
    static constexpr size_t NPOS = View::NPOS;

    //! 默认初始化为空串
    String();

    //! 将字符串初始化为n个相同字符
    explicit String(CodePoint cp, size_t count = 1);
    //! 从视图构造字符串
    String(const View &view);
    //! 用给定的一段码元初始化字符串
    String(const CodeUnit *beg, size_t len);
    //! 用给定的一段码元初始化字符串
    String(const CodeUnit *beg, const CodeUnit *end);
    //! 以另一个字符串中的一段码元初始化字符串
    String(const Self &copyFrom, size_t begIdx, size_t endIdx);

    //! 从C-style字符串中初始化，缺省认为输入串使用UTF-8编码
    String(const char *cstr, NativeCharset cs = NativeCharset::UTF8);
    //! 从std::string中初始化，缺省认为输入串使用UTF-8编码
    String(const std::string &cppStr, NativeCharset cs = NativeCharset::UTF8);

    //! 从C-style宽字符串中初始化，缺省认为输入串使用WUTF编码
    String(const wchar_t *cstr, NativeCharset cs = NativeCharset::WUTF);
    //! 从std::wstring中初始化，缺省认为输入串使用WUTF编码
    String(const std::wstring &cppStr, NativeCharset cs = NativeCharset::WUTF);

    String(const Self &copyFrom);
    String(Self &&moveFrom) noexcept;

    //! 从使用其他编码方案的字符串中初始化，会自动进行编码转换
    template<typename OCS, std::enable_if_t<!std::is_same_v<CS, OCS>, int> = 0>
    explicit String(const StringView<OCS> &convertFrom);

    //! 从使用其他编码方案的字符串中初始化，会自动进行编码转换
    template<typename OCS, std::enable_if_t<!std::is_same_v<CS, OCS>, int> = 0>
    explicit String(const String<OCS> &convertFrom): Self(convertFrom.AsView()) {  }

    /** 默认析构 */
    ~String() = default;

    String<CS> &operator=(const Self &copyFrom);
    String<CS> &operator=(Self &&moveFrom) noexcept;

    /**
     * @brief 将整数转换为字符串
     * 
     * @param v 待转换的整数值
     * @param base 转换所使用的进制，缺省为10
     */
    static Self From(char               v, unsigned int base = 10);
    /** 见 String<CS>::From(char, unsigned int) */
    static Self From(signed char        v, unsigned int base = 10);
    /** 见 String::From(char, unsigned int) */
    static Self From(unsigned char      v, unsigned int base = 10);
    /** 见 String<CS>::From(char, unsigned int) */
    static Self From(short              v, unsigned int base = 10);
    /** 见 String<CS>::From(char, unsigned int) */
    static Self From(unsigned short     v, unsigned int base = 10);
    /** 见 String<CS>::From(char, unsigned int) */
    static Self From(int                v, unsigned int base = 10);
    /** 见 String<CS>::From(char, unsigned int) */
    static Self From(unsigned int       v, unsigned int base = 10);
    /** 见 String<CS>::From(char, unsigned int) */
    static Self From(long               v, unsigned int base = 10);
    /** 见 String<CS>::From(char, unsigned int) */
    static Self From(unsigned long      v, unsigned int base = 10);
    /** 见 String<CS>::From(char, unsigned int) */
    static Self From(long long          v, unsigned int base = 10);
    /** 见 String<CS>::From(char, unsigned int) */
    static Self From(unsigned long long v, unsigned int base = 10);

    /** 将浮点数转换为字符串 */
    static Self From(float v);
    /** 将浮点数转换为字符串 */
    static Self From(double v);

    /**
     * @brief 将字符串转换为整数值
     * 
     * @param base 转换所使用的进制，缺省为10
     */
    template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    T Parse(unsigned int base = 10) const { return AsView().template Parse<T>(base); }

    //! 将字符串转换为浮点数
    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    T Parse() const { return AsView().template Parse<T>(); }

    //! 取得自己的视图
    View AsView() const;

    //! 取得自己所存储的码元指针和数量
    const CodeUnit *Data() const;
    std::pair<const CodeUnit*, size_t> DataAndLength() const;

    //! 取得码元数量
    size_t Length() const;
    //! 是否是空串
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

    CharRange<CS> Chars() const &  { return CharRange(begin(), end()); }
    CharRange<CS> Chars() const && { return CharRange(*this, begin(), end()); }

    std::pair<const CodeUnit*, const CodeUnit*> BeginAndEnd() const { return storage_.BeginAndEnd(); }

    //! 将其他字符串追加到末尾
    template<typename RHS>
    Self &operator+=(const RHS &rhs) { return *this = *this + rhs; }

    //! 二进制序列化
    bool Serialize(BinarySerializer &serialize) const;

    //! 二进制反序列化
    bool Deserialize(BinaryDeserializer &deserialize);
};

//! 复读机
template<typename CS>
String<CS> operator*(const String<CS> &lhs, size_t rhs);
//! 复读机
    template<typename CS>
String<CS> operator*(size_t L, const String<CS> &R) { return R * L; }
//! 复读机
template<typename CS>
String<CS> operator*(const StringView<CS> &L, size_t R) { return L.AsString() * R; }
//! 复读机
template<typename CS>
String<CS> operator*(size_t L, const StringView<CS> &R) { return R * L; }

#define AGZ_WRAP_STR_COMP(LHS, RHS, LOP, ROP) \
    template<typename CS> String<CS> operator+(const LHS lhs, const RHS rhs) { return (LOP) + (ROP); } \
    template<typename CS> bool operator==(const LHS lhs, const RHS rhs) { return (LOP) == (ROP); } \
    template<typename CS> bool operator!=(const LHS lhs, const RHS rhs) { return (LOP) != (ROP); } \
    template<typename CS> bool operator< (const LHS lhs, const RHS rhs) { return (LOP) < (ROP); } \
    template<typename CS> bool operator> (const LHS lhs, const RHS rhs) { return (LOP) > (ROP); } \
    template<typename CS> bool operator<=(const LHS lhs, const RHS rhs) { return (LOP) <= (ROP); } \
    template<typename CS> bool operator>=(const LHS lhs, const RHS rhs) { return (LOP) >= (ROP); }

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

/**
 * @brief 用于加速大量字符串的拼接
 * 
 * @note 线程不安全
 */
template<typename CS>
class StringBuilder
{
    mutable std::list<String<CS>> strs_;

public:

    using Self = StringBuilder<CS>;

    /**
     * @brief 将给定视图的内容追加到正在构建的字符串末尾
     * 
     * @param view 被追加的视图
     * @param n 追加重复次数，缺省为1
     */
    Self &Append(const StringView<CS> &view, size_t n = 1);
    /**
     * @brief 将给定字符串追加到正在构建的字符串末尾
     *
     * @param str 被追加的字符串
     * @param n 追加重复次数，缺省为1
     */
    Self &Append(const String<CS> &str, size_t n = 1);

    //! 将给定视图的内容追加到正在构建的字符串末尾
    Self &operator<<(const StringView<CS> &view);
    //! 将给定字符串追加到正在构建的字符串末尾
    Self &operator<<(const String<CS> &view) { return *this << view.AsView(); }

    //! 取得被构建的字符串
    String<CS> Get() const;

    //! 是否包含任何正在被构建的字符串
    bool Empty() const { return strs_.empty(); }

    //! 清空正在被构建的字符串
    void Clear() { strs_.clear(); }
};

/**
 * @brief 字符编码转换器
 */
class CharsetConvertor
{
public:

    //! 将以SCS方案编码的字符串视图转换为以DCS方案编码的字符串
    template<typename DCS, typename SCS>
    static String<DCS> Convert(const typename String<SCS>::View &src);

    //! 将以SCS方案编码的字符串转换为以DCS方案编码的字符串
    template<typename DCS, typename SCS>
    static String<DCS> Convert(const String<SCS> &src) { return Convert<DCS, SCS>(src.AsView()); }
};

/**
 * @brief 统一的将对象转为String的接口
 * 
 * - 若obj.ToString()类型和String<CS>相同，则返回obj.ToString()
 * - 否则，若String<CS>::From(obj)合法，则返回其结果
 * - 否则，若stringBuilder << obj合法，则用stringBuilder进行转换
 * - 否则报错
*/
template<typename CS, typename T>
String<CS> ToString(const T &obj);

} // namespace AGZ::StrImpl

namespace AGZ {

//! @copydoc StrImpl::NativeCharset
using NativeCharset = StrImpl::NativeCharset;

//! @copydoc StrImpl::String<CS>
template<typename CS>
using String = StrImpl::String<CS>;
//! @copydoc StrImpl::StringView<CS>
template<typename CS>
using StringView = StrImpl::StringView<CS>;

using Str8  = String<UTF8<>>;  ///< 以UTF-8编码的字符串
using Str16 = String<UTF16<>>; ///< 以UTF-16编码的字符串
using Str32 = String<UTF32<>>; ///< 以UTF-32编码的字符串
using AStr  = String<ASCII<>>; ///< 以ASCII编码的字符串
using WStr  = String<WUTF>;    ///< 以宽字符编码（平台相关）的字符串
using PStr  = String<PUTF>;    ///< 平台缺省使用的字符串

using StrImpl::ToString;

template<typename T> auto ToStr8 (const T &obj) { return ToString<UTF8<>,  T>(obj); }
template<typename T> auto ToStr16(const T &obj) { return ToString<UTF16<>, T>(obj); }
template<typename T> auto ToStr32(const T &obj) { return ToString<UTF32<>, T>(obj); }
template<typename T> auto ToAStr (const T &obj) { return ToString<ASCII<>, T>(obj); }
template<typename T> auto ToWStr (const T &obj) { return ToString<WUTF,    T>(obj); }
template<typename T> auto ToPStr (const T &obj) { return ToString<PUTF,    T>(obj); }

using StdPStr = decltype(std::declval<Str8>().ToPlatformString());

using StrView8  = StringView<UTF8<>>;  ///< 以UTF-8编码的字符串视图
using StrView16 = StringView<UTF16<>>; ///< 以UTF-16编码的字符串视图
using StrView32 = StringView<UTF32<>>; ///< 以UTF-32编码的字符串视图
using AStrView  = StringView<ASCII<>>; ///< 以ASCII编码的字符串视图
using WStrView  = StringView<WUTF>;    ///< 以宽字符编码（平台相关）的字符串视图
using PStrView  = StringView<PUTF>;    ///< 平台缺省使用的字符串视图

using CSConv = StrImpl::CharsetConvertor;

template<typename CS>
using StringBuilder = StrImpl::StringBuilder<CS>;

using Str8Builder  = StringBuilder<UTF8<>>;
using Str16Builder = StringBuilder<UTF16<>>;
using Str32Builder = StringBuilder<UTF32<>>;
using AStrBuilder  = StringBuilder<ASCII<>>;
using WStrBuilder  = StringBuilder<WUTF>;
using PStrBuilder  = StringBuilder<PUTF>;

template<typename CS>
using CodePointRange = StrImpl::CodePointRange<CS>;

template<typename CS>
using CharRange = typename StrImpl::StringView<CS>::CharRange;

} // namespace AGZ

namespace std
{
    template<typename CS>
    struct hash<AGZ::String<CS>>
    {
        size_t operator()(const AGZ::String<CS>& s) const noexcept
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
        size_t operator()(const AGZ::StringView<CS>& s) const noexcept
        {
            size_t ret = 0;
            for(size_t i = 0; i < s.Length(); i++)
                ret = 65599 * ret + s.Data()[i];
            return ret ^ (ret >> 16);
        }
    };
}
