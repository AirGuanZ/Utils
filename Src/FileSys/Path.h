#pragma once

#include <vector>

#include "../Misc/Common.h"
#include "../Utils/Range.h"
#include "../String/String.h"

AGZ_NS_BEG(AGZ)

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

    bool HasFilename() const { return !filename_.Empty(); }

    StrView GetFilename() const
    {
        AGZ_ASSERT(HasFilename());
        return filename_;
    }

    void SetFilename(const StrView &filename) { filename_ = filename; }

    void SetFilename(const Str &filename = Str()) { filename_ = filename; }

    Str GetExtension() const
    {
        AGZ_ASSERT(HasFilename());
        auto m = ExtRegex().Match(filename_);
        return m ? Str(m(1, 2)) : Str();
    }

    void SetExtension(const Str &ext)
    {
        SetExtension(ext.AsView());
    }

    void SetExtension(const StrView &ext)
    {
        AGZ_ASSERT(HasFilename());
        auto m = ExtRegex().Match(filename_);
        if(m)
            filename_ = m(0, 1) + ext;
        else
            filename_ += "." + ext;
    }

private:

    const Regexp &ExtRegex() const
    {
        static const Regexp regex("&.*\\.&<^\\.\\.>+&");
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

AGZ_NS_END(AGZ)
