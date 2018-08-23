#pragma once

#include <vector>

#include "../Misc/Common.h"
#include "../String/String.h"

AGZ_NS_BEG(AGZ)

template<typename CS = WUTF>
class Path
{
public:
    using Str      = String<CS>;
    using StrView  = StringView<CS>;
    using Interval = std::pair<size_t, size_t>;
    using Iterator = std::vector<Str>::const_iterator;

    enum SeperatorStyle
    {
        Unix,    // '/' only
        Windows, // '\' or '/'
#ifdef AGZ_OS_WIN32
        Native = Windows,
#else
        Native = Unix,
#endif
    };

    Path() : abs_(false) { }

    // A/B  => regular
    // A/B/ => directory
    template<typename CS>
    Path(const String<CS> &path,
         SeperatorStyle style = Native)
         : Path<CS>(path, true, style)
    {

    }

    // A/B is recognized as a regular file by default.
    // Specify hasFilename == false to force it a directory path.
    template<typename CS>
    Path(const String<CS> &path, bool hasFilename,
         SeperatorStyle style = Native)
    {
        Str seps[2] = { "/" };
        if(style == SeperatorStyle::Windows)
            seps[1] = "\\";

        StringBuilder<CS> dirBuilder;
        bool isLastSep = false;
        for(auto c : path.Chars())
        {
            bool isLastSep = c == seps[1] || c == seps[2];

            if(isLastSep)
            {
                if(!dirBuilder.Empty())
                {
                    dirs_.push_back(dirBuilder.Get());
                    dirBuilder.Clear();
                }
            }
            else
                dirBuilder << c;
        }

        if(!isLastSep && hasFilename)
        {
            AGZ_ASSERT(!dirBuilder.Empty());
            filename_ = dirBuilder.Get();
        }
        else if(!dirBuilder.Empty())
            dirs_.push_back(dirBuilder.Get());

        abs_ = style == SeperatorStyle::Windows ?
               path.Find(":") != String<CS>::NPOS :
               path.Length() && path[0] == '/';
    }

    Path(Path &&moveFrom) noexcept
        : dirs_(std::move(moveFrom.dirs_)),
          filename_(std::move(moveFrom.filename_)),
          abs_(moveFrom.abs_)
    {

    }

    Path(const Path &copyFrom)            = default;
    Path &operator=(const Path &copyFrom) = default;
    ~Path()                               = default;

    Path &operator=(Path &&moveFrom) noexcept
    {
        dirs_ = std::move(moveFrom.dirs_);
        filename_ = std::move(moveFrom.filename_);
        abs_ = moveFrom.abs_;
        return *this;
    }

    void Clear()
    {
        dirs_.clear();
        filename_ = "";
    }

    bool Empty() const
    {
        return dirs_.empty() && filename_.Empty();
    }

    bool IsAbsolute() const
    {
        return abs_;
    }

    bool HasFilename() const
    {
        return !filename_.Empty();
    }

    void RemoveFilename()
    {
        filename_ = "";
    }

    void SetFilename(const StrView &filename)
    {
        filename_ = filename;
    }

    void SetFilename(const Str &filename)
    {
        filename_ = filename;
    }

    StrView GetFilename() const
    {
        AGZ_ASSERT(HasFilename());
        return filename_;
    }

    Str GetExtension() const
    {
        AGZ_ASSERT(HasFilename());
        auto m = Regex8(".*\\.&.*&").Match(filename_);
        return m ? Str(m(0, 1)) : Str("");
    }

    size_t GetDirectoryCount() const
    {
        return dirs_.size();
    }

    Str GetDirectory(SeperatorStyle style = Native) const
    {
        switch(style)
        {
        case Windows:
            return Str("\\").Join(dirs_);
        case Unix:
            return Str("/").Join(dirs_);
        default:
            Unreachable();
        }
    }

    StrView GetDirectory(size_t idx) const
    {
        AGZ_ASSERT(idx < dirs_.size());
        return dirs_[idx];
    }

    void SetExtension(const StrView &ext)
    {
        AGZ_ASSERT(HasFilename());
        auto m = Regex8("&.*\\.&.*").Match(filename_);
        filename_ = m ? m(0, 1) + ext : filename_ + ext;
    }

    void SetExtension(const Str &ext)
    {
        SetExtension(ext.AsView());
    }

    Str GetStr(SeperatorStyle style = Native) const
    {
        Str sep = style == SeperatorStyle::Windows ?
                  "\\" : "/";
        StringBuilder<CS> builder;
        builder << sep.Join(dirs_) << sep;
        if(HasFilename())
            builder << filename_;
        return builder.Get();
    }

    Path operator+(const Path &rhs) const
    {
        Path ret = *this;
        if(hasFilename() || rhs.IsAbsolute())
            throw ArgumentException("Invalid path concatenation");
        for(auto &d : rhs)
            ret.dirs_.push_back(d);
        ret.filename_ = rhs.filename_;
        return std::move(ret);
    }

    Path &operator+=(const Path &rhs)
    {
        return *this = *this + rhs;
    }

    Iterator begin() const
    {
        return dirs_.begin();
    }

    Iterator end() const
    {
        return dirs_.end();
    }

private:

    std::vector<Str> dirs_;
    Str filename_;
    bool abs_;
};

AGZ_NS_END(AGZ)
