﻿#pragma once

#include "../Misc/Common.h"

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

Option<FileTime> File::GetLastWriteTime(const Str8 &filename)
{
    WIN32_FIND_DATA findData;

    auto hFind = FindFirstFileW(filename.ToStdWString().c_str(), &findData);
    if(hFind == INVALID_HANDLE_VALUE)
        return None;
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

Str8 File::GetWorkingDirectory()
{
    DWORD len = GetCurrentDirectory(0, NULL);
    std::vector<wchar_t> buf(len + 1);

    if((len = GetCurrentDirectory(len, buf.data())))
    {
        if(buf[len - 1] != '\\' && buf[len - 1] != '/')
        {
            buf[len] = '\\';
            return Str8(WStr(buf.data(), len + 1));
        }
        return Str8(WStr(buf.data(), len));
    }
    throw OSException("Failed to get the working directory");
}

bool File::CreateDirectoryRecursively(const Str8 &directory)
{
    return std::filesystem::create_directories(directory.ToStdString());
}

bool File::IsRegularFile(const Str8 &filename)
{
    return std::filesystem::is_regular_file(filename.ToStdString());
}

bool File::DeleteFile(const Str8 &filename)
{
    auto s = filename.ToStdString();
    return IsRegularFile(s) && std::filesystem::remove(s);
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

Option<FileTime> File::GetLastWriteTime(const Str8 &filename)
{
    struct stat buf;
    if(stat(filename.ToStdString().c_str(), &buf))
        return None;

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

Str8 File::GetWorkingDirectory()
{
    char buf[PATH_MAX + 1];
    if(!getcwd(buf, PATH_MAX))
        throw OSException("Failed to get the working directory");
    
    auto w = Str8(buf);
    return w.EndsWith("/") ? std::move(w) : w + "/";
}

bool File::CreateDirectoryRecursively(const Str8 &directory)
{
    Path8 p(directory, false);
    AGZ_ASSERT(p.IsDirectory());
    size_t sc = p.GetSectionCount();
    for(size_t i = 1; i <= sc; ++i)
        mkdir(p.GetPrefix(i).ToStr().ToPlatformString().c_str(), S_IRWXU);
    return true;
}

bool File::IsRegularFile(const Str8 &filename)
{
    struct stat buf;
    if(stat(filename.ToStdString().c_str(), &buf))
        return false;
    return static_cast<bool>(S_ISREG(buf.st_mode));
}

bool File::DeleteFile(const Str8 &filename)
{
    if(!IsRegularFile(filename))
        return false;
    return !remove(filename.ToStdString().c_str());
}

} // namespace AGZ::FileSys

#else

#error "OS File unimplementated"

#endif

#endif // #ifdef AGZ_FILE_IMPL
