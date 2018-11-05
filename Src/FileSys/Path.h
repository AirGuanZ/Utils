#pragma once

#include <vector>

#include "../Misc/Common.h"
#include "../Misc/Exception.h"
#include "../Utils/Platform.h"
#include "../Utils/Range.h"
#include "../Utils/String.h"

namespace AGZ::FileSys {

template<typename CS>
class Path
{
public:

    using Charset = CS;
    using Str     = String<CS>;
    using StrView = StringView<CS>;
    using Regexp  = Regex<CS>;
    using Self    = Path<CS>;

    enum SeperatorStyle
    {
        Linux,
        Windows,
#if defined(AGZ_OS_WIN32)
        Native = Windows,
#elif defined(AGZ_OS_LINUX)
        Native = Linux,
#endif
    };

    Path() : abs_(false) { }

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

    Path(const StrView &s, SeperatorStyle style = Native)
        : Path(s, true, style)
    {

    }

    Path(const Str &s, bool mayHasFilename, SeperatorStyle style = Native)
        : Path(s.AsView(), mayHasFilename, style)
    {

    }

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

    bool IsAbsolute() const { return abs_; }

    bool IsRelative() const { return !IsAbsolute(); }

    bool HasFilename() const { return !filename_.Empty(); }

    bool IsDirectory() const { return !HasFilename(); }

    bool HasParent() const
    {
        if(HasFilename())
            return !dirs_.empty();
        return dirs_.size() >= 2;
    }

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

    StrView GetFilename() const
    {
        AGZ_ASSERT(HasFilename());
        return filename_;
    }

    Str ToStr(SeperatorStyle style = Native)
    {
        return GetDirectoryStr(style) + filename_;
    }

    Str GetDirectoryStr(SeperatorStyle style = Native)
    {
        auto s = style == Windows ? "\\" : "/";
        return Str(s).Join(dirs_) + s;
    }

    Self &SetFilename(const StrView &filename)
    {
        filename_ = filename;
        return *this;
    }

    Self &SetFilename(const Str &filename = Str())
    {
        filename_ = filename;
        return *this;
    }

    Str GetExtension() const
    {
        AGZ_ASSERT(HasFilename());
        auto m = ExtRegex().Match(filename_);
        return m ? Str(m(1, 2)) : Str();
    }

    Self &SetExtension(const Str &ext)
    {
        SetExtension(ext.AsView());
        return *this;
    }

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

    Self &ToAbsolute(SeperatorStyle style = Native)
    {
        if(IsAbsolute())
            return *this;
        return *this = Self(Platform::GetWorkingDirectory(), false, style)
                     + *this;
    }

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

    Self &ToDirectory()
    {
        if(HasFilename())
            filename_ = Str();
        return *this;
    }

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

    bool operator!=(const Self &rhs) const
    {
        return !(*this == rhs);
    }

    Self operator+(const Self &rhs) const
    {
        Self ret = *this;
        ret.Append(rhs);
        return ret;
    }

    Self &operator+=(const Self &rhs) const
    {
        return Append(rhs);
    }

private:

    const Regexp &ExtRegex() const
    {
        static const Regexp regex("&.*\\.&@{!\\.}+&");
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
