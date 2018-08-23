#include "../Misc/Common.h"
#include "Platform.h"

#ifdef AGZ_OS_WIN32

#include <vector>
#include <Windows.h>

#elif defined(AGZ_OS_LINUX)

#include <unistd.h>

#else

#warning "OS Platform unimplemented"

#endif

AGZ_NS_BEG(AGZ)

WStr Platform::GetWorkingDirectory()
{
#ifdef AGZ_OS_WIN32
    DWORD len = GetCurrentDirectory(0, NULL);
    std::vector<wchar_t> buf(len);
    if((len = GetCurrentDirectory(len, buf.data())))
        return WStr(buf.data());
    throw OSException("Failed to get the working directory");
#elif defined(AGZ_OS_LINUX)
    char buf[PATH_MAX];
    if(!getcwd(buf, PATH_MAX))
        throw OSException("Failed to get the working directory");
    return WStr(buf);
#else
    throw OSException("Failed to get the working directory");
#endif
}

AGZ_NS_END(AGZ)
