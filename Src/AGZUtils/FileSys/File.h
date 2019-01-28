#pragma once

#include <cstring>
#include <filesystem>
#include <string>

#include "../Misc/Common.h"
#include "AGZUtils/String/StdStr.h"

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
    static std::string GetWorkingDirectory();

    //! 创建指定目录
    static bool CreateDirectoryRecursively(std::string_view directory);

    //! 查询一个路径是否是regular file
    static bool IsRegularFile(std::string_view filename);

    //! 删除指定文件
    static bool DeleteRegularFile(std::string_view filename);

    /**
     * @brief 取得给定路径下的所有directory和regular file
     */
    template<typename TOutputIterator>
    void GetFilesInDirectory(std::string_view dir, TOutputIterator outputIterator);
};

template<typename TOutputIterator>
void File::GetFilesInDirectory(std::string_view dir, TOutputIterator outputIterator)
{
    AGZ_ASSERT(std::filesystem::is_directory(dir));
    for(auto &p : std::filesystem::directory_iterator(dir))
    {
        if(!p.is_regular_file() && !p.is_directory())
            continue;
#ifdef AGZ_OS_WIN32
        outputIterator = INV_WIDEN(p.path().wstring());
#else
        outputIterator = p.path().string();
#endif
        ++outputIterator;
    }
}

} // namespace AGZ::FileSys
