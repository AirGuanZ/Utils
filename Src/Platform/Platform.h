#pragma once

#include "../Utils/String.h"

namespace AGZ {

/**
 * @brief 一些平台相关的功能
 */
class Platform
{
public:

	/**
	 * 取得当前工作目录
	 */
    static WStr GetWorkingDirectory();
};

} // namespace AGZ

#if defined(AGZ_PLATFORM_IMPL)

#include "../Misc/Exception.h"

#if defined(AGZ_OS_WIN32)

#include <vector>
#include <Windows.h>

#elif defined(AGZ_OS_LINUX)

#include <limits.h>
#include <unistd.h>

#else

#warning "OS Platform unimplemented"

#endif

namespace AGZ {

WStr Platform::GetWorkingDirectory()
{
#if defined(AGZ_OS_WIN32)

    DWORD len = GetCurrentDirectory(0, NULL);
    std::vector<wchar_t> buf(len + 1);

    if((len = GetCurrentDirectory(len, buf.data())))
    {
        if(buf[len - 1] != '\\' && buf[len - 1] != '/')
        {
            buf[len] = '\\';
            return WStr(buf.data(), len + 1);
        }
        return WStr(buf.data(), len);
    }
    throw OSException("Failed to get the working directory");

#elif defined(AGZ_OS_LINUX)

    char buf[PATH_MAX + 1];
    if(!getcwd(buf, PATH_MAX))
        throw OSException("Failed to get the working directory");
    
    auto w = WStr(buf);
    return w.EndsWith(L"/") ? std::move(w) : w + L"/";

#else

    throw OSException("Failed to get the working directory");

#endif
}

} // namespace AGZ

#endif
