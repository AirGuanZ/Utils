#pragma once

#include "../Misc/Common.h"
#include "../Utils/String.h"

AGZ_NS_BEG(AGZ)

class Platform
{
public:

    static WStr GetWorkingDirectory();
};

AGZ_NS_END(AGZ)

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

AGZ_NS_BEG(AGZ)

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

AGZ_NS_END(AGZ)

#endif
