#pragma once

#include "../Misc/Common.h"
#include "../Utils/String.h"

namespace AGZ::FileSys {

struct FileTime
{
    int32_t year;
    int32_t month;
    int32_t day;
    int32_t hour;
    int32_t minute;
    int32_t second;

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
    static Option<FileTime> GetLastWriteTime(const PStr &filename);

    //! 取得当前工作目录（绝对路径）
    static WStr GetWorkingDirectory();

    //! 创建指定目录
    static bool CreateDirectory(const PStr &directory);
};

} // namespace AGZ::FileSys
