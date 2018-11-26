#pragma once

#include <vector>

#include "../Misc/Common.h"
#include "../Misc/Exception.h"
#include "../Utils/Platform.h"
#include "../Utils/Range.h"
#include "../Utils/String.h"

namespace AGZ::FileSys {

/**
 * @brief 可在Windows和*nix下使用的路径类封装
 */
template<typename CS>
class Path
{
public:

    using Charset = CS;                ///< 所使用的字符编码方案
    using Str     = String<CS>;        ///< 所使用的字符串类型
    using StrView = StringView<CS>;    ///< 所使用的字符串视图类型
    using Self    = Path<CS>;        ///< 自身类型

    /** 不同操作系统下的路径风格 */
    enum SeperatorStyle
    {
        Linux,                ///< *nix风格，以“/”作为唯一分隔符，绝对路径以根目录“/”开头
        Windows,            ///< Window风格，以“/”或“\”作为分隔符，绝对路径以盘符开头
#if defined(AGZ_OS_WIN32)
        Native = Windows,    ///< 由编译器自动根据所在平台判断应该使用哪种路径风格
#else
        Native = Linux,        ///< 由编译器自动根据所在平台判断应该使用哪种路径风格
#endif
    };

    Path() : abs_(false) { }

    /**
     * @param s 用字符串表示的路径
     * @param mayHasFilename 该值为true时，形如“.../A/B”的s被认为是文件名，否则被认为是目录名
     * @param style 用哪种路径风格来解析s
     * 
     * @exception ArgumentException 路径格式不合法时抛出
     */
    Path(const StrView &s, bool mayHasFilename, SeperatorStyle style = Native)
    {
        bool hasFilename;

        if(style == Windows)
        {
            dirs_ = s.Split(std::vector<Str>{ "\\", "/" })
                  | Map([](const StrView &v) { return v.AsString(); })
                  | Collect<std::vector<Str>>();
            abs_ = s.Find(":") != StrView::NPOS;
            hasFilename = mayHasFilename & !(s.EndsWith("\\")
                                          || s.EndsWith("/"));
        }
        else
        {
            dirs_ = s.Split("/")
                  | Map([](const StrView &v) { return v.AsString(); })
                  | Collect<std::vector<Str>>();
            abs_ = s.StartsWith("/");
            hasFilename = mayHasFilename & !s.EndsWith("/");
        }

        if(hasFilename)
        {
            if(dirs_.empty())
                throw ArgumentException("Invalid path: empty filename");
            filename_ = std::move(dirs_.back());
            dirs_.pop_back();
        }
    }

    /** 同 Path::Path(const StrView&, bool, SeperatorStyle) ，取mayHasFilename为true */
    Path(const StrView &s, SeperatorStyle style = Native)
        : Path(s, true, style)
    {

    }

    //! @copydoc Path::Path(const StrView&, bool, SeperatorStyle)
    Path(const Str &s, bool mayHasFilename, SeperatorStyle style = Native)
        : Path(s.AsView(), mayHasFilename, style)
    {

    }

    /** 同 Path::Path(const Str&, bool, SeperatorStyle) ，取mayHasFilename为true */
    Path(const Str &s, SeperatorStyle style = Native)
        : Path(s.AsView(), style)
    {

    }

    Path(Self &&moveFrom) noexcept
        : dirs_(std::move(moveFrom.dirs_)),
          filename_(std::move(moveFrom.filename_)),
          abs_(moveFrom.abs_)
    {
        
    }

    Self &operator=(Self &&moveFrom) noexcept
    {
        dirs_ = std::move(moveFrom.dirs_);
        filename_ = std::move(moveFrom.filename_);
        abs_ = moveFrom.abs_;
        return *this;
    }

    Path(const Self &copyFrom)            = default;
    Self &operator=(const Self &copyFrom) = default;
    ~Path()                               = default;

    /**
     * 是否是一个绝对路径
     * 
     * @note 空路径被认为是相对路径
     */
    bool IsAbsolute() const { return abs_; }

    /**
     * 是否是一个相对路径
     * 
     * @note 空路径被认为是相对路径
     */
    bool IsRelative() const { return !IsAbsolute(); }

    /** 是否是一个常规文件 */
    bool HasFilename() const { return !filename_.Empty(); }

    /** 是否是一个目录 */
    bool IsDirectory() const { return !HasFilename(); }

    /** 是否有父目录 */
    bool HasParent() const
    {
        if(HasFilename())
            return !dirs_.empty();
        return dirs_.size() >= 2;
    }

    /** 是否是另一个路径的前缀 */
    bool IsPrefixOf(const Self &parent) const
    {
        if(HasFilename())
            return false;
        if(dirs_.size() > parent.dirs_.size())
            return false;
        for(size_t i = 0; i < dirs_.size(); ++i)
        {
            if(dirs_[i] != parent.dirs_[i])
                return false;
        }
        return true;
    }

    /**
     * 常规文件的文件名本身
     * 
     * @warning 对目录路径调用该方法会造成UB
     */
    StrView GetFilename() const
    {
        AGZ_ASSERT(HasFilename());
        return filename_;
    }

    /**
     * 本路径的字符串表示
     * 
     * @param style 转换时使用的路径风格，缺省由编译器自动判断
     */
    Str ToStr(SeperatorStyle style = Native) const
    {
        return GetDirectoryStr(style) + filename_;
    }

    /**
     * 本路径的目录的字符串表示
     * 
     * @param style 转换时使用的路径风格，缺省由编译器自动判断
     * 
     * @return 若*this为目录，将返回字符串表示的目录；若*this为文件，会返回它所在的目录的字符串
     */
    Str GetDirectoryStr(SeperatorStyle style = Native) const
    {
        auto s = style == Windows ? "\\" : "/";
        return Str(s).Join(dirs_) + s;
    }

    /**
     * 对目录调用该方法会将其转换为文件路径，对文件路径调用该方法则会在不改变父目录的情况下更换文件名。
     * 
     * @warning 若filename不是一个合法的文件名（如“A/B”），则会造成后续操作UB
     */
    Self &SetFilename(const StrView &filename)
    {
        filename_ = filename;
        return *this;
    }

    //! @copydoc Path::SetFilename(const StrView &)
    Self &SetFilename(const Str &filename = Str())
    {
        filename_ = filename;
        return *this;
    }

    /**
     * 取得文件扩展名，不包含“.”
     * 
     * @warning *this不是文件路径会造成UB
     */
    Str GetExtension() const
    {
        AGZ_ASSERT(HasFilename());
        auto m = ExtRegex().Match(filename_);
        return m ? Str(m(1, 2)) : Str();
    }

    /**
     * 设置文件扩展名
     * 
     * @param ext 新扩展名，如“txt”、“rar”等
     * 
     * @warning *this不是文件路径会造成UB
     */
    Self &SetExtension(const Str &ext)
    {
        SetExtension(ext.AsView());
        return *this;
    }

    //! @copydoc Path::SetExtension(const Str &)
    Self &SetExtension(const StrView &ext)
    {
        AGZ_ASSERT(HasFilename());
        auto m = ExtRegex().Match(filename_);
        if(m)
            filename_ = m(0, 1) + ext;
        else
            filename_ += "." + ext;
        return *this;
    }

    /**
     * 将一个相对路径追加到本路径后方
     * 
     * @param tail 被追加的相对路径
     * 
     * @exception ArgumentException 本路径为文件路径或追加路径为绝对路径时抛出
     */
    Self &Append(const Self &tail)
    {
        if(HasFilename())
            throw ArgumentException("Append: left operand with filename");
        if(tail.IsAbsolute())
            throw ArgumentException("Append: absolute right operand");

        dirs_.reserve(dirs_.size() + tail.dirs_.size());
        for(const auto &s : tail.dirs_)
            dirs_.push_back(s);
        filename_ = tail.filename_;

        return *this;
    }

    /**
     * 将本路径转换为绝对路径的结果
     * 
     * @note 若本路径已经是绝对路径，则返回值与本路径相同
     */
    Self &ToAbsolute(SeperatorStyle style = Native)
    {
        if(IsAbsolute())
            return *this;
        return *this = Self(Platform::GetWorkingDirectory(), false, style)
                     + *this;
    }

    /**
     * 将本路径转换为相对路径的结果
     * 
     * @note 若本路径已经是相对路径，则返回值与本路径相同
     */
    Self &ToRelative(SeperatorStyle style = Native)
    {
        if(IsRelative())
            return *this;

        Self wd(Platform::GetWorkingDirectory(), false, style);
        if(wd.IsPrefixOf(*this))
        {
            dirs_ = std::vector<Str>(dirs_.begin() + wd.dirs_.size(),
                                     dirs_.end());
        }

        return *this;
    }

    /**
     * 将本路径转换为目录的结果（去掉文件名）
     * 
     * @note 若本路径已经是目录，则返回值与本路径相同
     */
    Self &ToDirectory()
    {
        if(HasFilename())
            filename_ = Str();
        return *this;
    }

    /**
     * 返回本路径的父目录
     * 
     * 若本路径为目录，则返回其上一级目录；若本路径为文件路径，则返回文件所处目录。
     */
    Self &ToParent()
    {
        if(HasFilename())
            filename_ = Str();
        else
        {
            if(!dirs_.empty())
                dirs_.pop_back();
        }
        return *this;
    }

    /** 两个路径严格相等 */
    bool operator==(const Self &rhs) const
    {
        if(dirs_.size() != rhs.dirs_.size())
            return false;
        for(size_t i = 0; i < dirs_.size(); ++i)
        {
            if(dirs_[i] != rhs.dirs_[i])
                return false;
        }
        return filename_ == rhs.filename_;
    }

    /** 两个路径间存在差异 */
    bool operator!=(const Self &rhs) const
    {
        return !(*this == rhs);
    }

    //! @copydoc Path::Append(const Self &)
    Self operator+(const Self &rhs) const
    {
        Self ret = *this;
        ret.Append(rhs);
        return ret;
    }

    /** 将rhs以 Path<CS>::Append 的方式追加到本路径后方  */
    Self &operator+=(const Self &rhs) const
    {
        return Append(rhs);
    }

private:

    using Regexp = Regex<CS>;

    const Regexp &ExtRegex() const
    {
        static thread_local const Regexp regex("&.*\\.&@{!\\.}+&");
        return regex;
    }

    std::vector<Str> dirs_;
    Str filename_;
    bool abs_;
};

using Path8  = Path<UTF8<>>;
using Path16 = Path<UTF16<>>;
using Path32 = Path<UTF32<>>;
using WPath  = Path<WUTF>;
using APath  = Path<ASCII<>>;

} // namespace AGZ::FileSys
