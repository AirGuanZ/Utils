#pragma once

#include <cstring>

#include "../Misc/Common.h"
#include "../Utils/String.h"

namespace AGZ::FileSys {

/**
 * @brief 文件系统时间戳
 */
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
    static std::optional<FileTime> GetLastWriteTime(std::string_view filename);

    //! 取得当前工作目录（绝对路径）
    static Str8 GetWorkingDirectory();

    //! 创建指定目录
    static bool CreateDirectoryRecursively(std::string_view directory);

    //! 查询一个路径是否是regular file
    static bool IsRegularFile(const Str8 &filename);

    //! 删除指定文件
    static bool DeleteRegularFile(const Str8 &filename);
};

} // namespace AGZ::FileSys
