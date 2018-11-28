#pragma once

#include "../Misc/Common.h"
#include "../Utils/String.h"

namespace AGZ::FileSys {

struct FileTime
{
    int32_t year   = 0;
    int32_t month  = 0;
    int32_t day    = 0;
    int32_t hour   = 0;
    int32_t minute = 0;
    int32_t second = 0;

    bool operator==(const FileTime &rhs) const { return std::memcmp(this, &rhs, sizeof(FileTime)) == 0; }
    bool operator!=(const FileTime &rhs) const { return !(*this == rhs); }
};

/**
 * @brief 文件属性查询
 */
class File
{
public:

    static FileTime GetCurrentFileTime();

    //! 取得用本地时间表示的最后修改时间
    static Option<FileTime> GetLastWriteTime(const Str8 &filename);

    //! 取得当前工作目录（绝对路径）
    static Str8 GetWorkingDirectory();

    //! 创建指定目录
    static bool CreateDirectoryRecursively(const Str8 &directory);
};

} // namespace AGZ::FileSys
