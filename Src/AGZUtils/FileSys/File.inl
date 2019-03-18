#pragma once

#include "../Misc/Common.h"
#include "../String/StdStr.h"

#ifdef AGZ_FILE_IMPL

#if defined(AGZ_CC_MSVC)

#include <filesystem>
#include <Windows.h>

namespace AGZ::FileSys {

FileTime File::GetCurrentFileTime()
{
    SYSTEMTIME local;
    GetLocalTime(&local);

    FileTime ret;
    ret.year   = local.wYear;
    ret.month  = local.wMonth;
    ret.day    = local.wDay;
    ret.hour   = local.wHour;
    ret.minute = local.wMinute;
    ret.second = local.wSecond;

    return ret;
}

std::optional<FileTime> File::GetLastWriteTime(std::string_view filename)
{
    WIN32_FIND_DATAW findData;

    auto hFind = FindFirstFileW(WIDEN(filename).c_str(), &findData);
    if(hFind == INVALID_HANDLE_VALUE)
        return std::nullopt;
    FindClose(hFind);

    SYSTEMTIME sysUTC, sysLocal;
    FileTimeToSystemTime(&findData.ftLastWriteTime, &sysUTC);
    SystemTimeToTzSpecificLocalTime(nullptr, &sysUTC, &sysLocal);

    FileTime ret;
    ret.year   = sysLocal.wYear;
    ret.month  = sysLocal.wMonth;
    ret.day    = sysLocal.wDay;
    ret.hour   = sysLocal.wHour;
    ret.minute = sysLocal.wMinute;
    ret.second = sysLocal.wSecond;

    return ret;
}

std::string File::GetWorkingDirectory()
{
    return INV_WIDEN(std::filesystem::current_path().wstring());
}

bool File::CreateDirectoryRecursively(std::string_view directory)
{
    return std::filesystem::create_directories(WIDEN(directory));
}

bool File::IsRegularFile(std::string_view filename)
{
    return std::filesystem::is_regular_file(std::filesystem::path(WIDEN(filename)));
}

bool File::DeleteRegularFile(std::string_view filename)
{
    return IsRegularFile(filename) && std::filesystem::remove(WIDEN(filename));
}

} // namespace AGZ::FileSys

#elif defined(AGZ_OS_LINUX)

#include <sys/stat.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

namespace AGZ::FileSys {

FileTime File::GetCurrentFileTime()
{
    auto t = time(nullptr);
    tm localTime;
    localtime_r(&t, &localTime);

    FileTime ret;
    ret.year   = localTime.tm_year;
    ret.month  = localTime.tm_mon;
    ret.day    = localTime.tm_mday;
    ret.hour   = localTime.tm_hour;
    ret.minute = localTime.tm_min;
    ret.second = localTime.tm_sec;

    return ret;
}

std::optional<FileTime> File::GetLastWriteTime(std::string_view filename)
{
    struct stat buf;
    if(stat(std::string(filename).c_str(), &buf))
        return std::nullopt;

    // localTime由内核自动分配，不要尝试手动释放它
    tm localTime;
    localtime_r(&buf.st_mtime, &localTime);

    FileTime ret;
    ret.year   = localTime.tm_year;
    ret.month  = localTime.tm_mon;
    ret.day    = localTime.tm_mday;
    ret.hour   = localTime.tm_hour;
    ret.minute = localTime.tm_min;
    ret.second = localTime.tm_sec;

    return ret;
}

std::string File::GetWorkingDirectory()
{
    return std::filesystem::current_path().string();
}

bool File::CreateDirectoryRecursively(std::string_view directory)
{
    return std::filesystem::create_directories(directory);
}

bool File::IsRegularFile(std::string_view filename)
{
    return std::filesystem::is_regular_file(filename);
}

bool File::DeleteRegularFile(std::string_view filename)
{
    if(!IsRegularFile(filename))
        return false;
    return std::filesystem::remove(filename);
}

} // namespace AGZ::FileSys

#else

#error "OS File unimplementated"

#endif

#endif // #ifdef AGZ_FILE_IMPL
