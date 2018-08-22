#pragma once

#include <vector>

#include "../Misc/Common.h"
#include "../String/String.h"

AGZ_NS_BEG(AGZ)

class Path
{
public:

    using Interval = std::pair<size_t, size_t>;
    using Iterator = std::vector<Str8>::const_iterator;

    enum SeperatorStyle
    {
        Windows, // '\' or '/'
        Unix,    // '/' only
#ifdef _WIN32
        Native = Windows,
#else
        Native = Unix,
#endif
    };

    Path() : abs_(false) { }

    template<typename CS>
    Path(const String<CS> &path,
         SeperatorStyle style = Native);

    template<typename CS>
    Path(const String<CS> &path, bool hasFilename,
         SeperatorStyle style = Native);

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

    void SetFilename(const StrView8 &filename)
    {
        filename_ = filename;
    }

    void SetFilename(const Str8 &filename)
    {
        filename_ = filename;
    }

    StrView8 GetFilename() const
    {
        AGZ_ASSERT(HasFilename());
        return filename_;
    }

    Str8 GetExtension() const
    {
        AGZ_ASSERT(HasFilename());
        auto m = Regex8(".*\\.&.*&").Match(filename_);
        return m ? Str8(m(0, 1)) : Str8("");
    }

    size_t GetDirectoryCount() const
    {
        return dirs_.size();
    }

    Str8 GetDirectory(SeperatorStyle style = Native) const
    {
        switch(style)
        {
        case Windows:
            return Str8("\\").Join(dirs_);
        case Unix:
            return Str8("/").Join(dirs_);
        default:
            Unreachable();
        }
    }

    StrView8 GetDirectory(size_t idx) const
    {
        AGZ_ASSERT(idx < dirs_.size());
        return dirs_[idx];
    }

    void SetExtension(const StrView8 &ext)
    {
        AGZ_ASSERT(HasFilename());
        auto m = Regex8("&.*\\.&.*").Match(filename_);
        filename_ = m ? m(0, 1) + ext : filename_ + ext;
    }

    void SetExtension(const Str8 &ext)
    {
        SetExtension(ext.AsView());
    }

    Str8 GetStr8(SeperatorStyle style = Native) const;
    WStr GetWStr(SeperatorStyle style = Native) const;

    void ToAbsolute(const Str8 &base = "");
    void ToAbsolute(const StrView8 &base);

    void ToRelative(const Str8 &base = "");
    void ToRelative(const StrView8 &base);

    Path operator+(const Path &rhs) const;

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

    std::vector<Str8> dirs_;
    Str8 filename_;
    bool abs_;
};

AGZ_NS_END(AGZ)
