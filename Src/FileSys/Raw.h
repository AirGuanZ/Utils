#pragma once

#include <fstream>
#include <iterator>

#include "../Misc/Common.h"
#include "../Utils/String.h"

AGZ_NS_BEG(AGZ::FileSys)

template<typename AllocFunc = void*(*)(size_t)>
std::pair<size_t, unsigned char*> ReadBinaryFileRaw(
    const WStr &filename, AllocFunc &&func = &(std::malloc))
{
    std::ifstream fin(filename.ToPlatformString().c_str(),
                      std::ios::in | std::ios::binary);
    if(!fin)
        return { 0, nullptr };

    fin.seekg(0, std::ios::end);
    auto len = fin.tellg();
    fin.seekg(0, std::ios::beg);

    auto buf = alloc_throw<unsigned char>(
        std::forward<AllocFunc>(func), static_cast<size_t>(len));
    fin.read(reinterpret_cast<char*>(buf), len);

    return { static_cast<size_t>(len), buf };
}

AGZ_FORCEINLINE void DefaultlyReleaseRawBinaryFileContent(unsigned char *ptr)
{
    std::free(ptr);
}

inline bool WriteBinaryFileRaw(
    const WStr &filename, const unsigned char *data, size_t len)
{
    std::ofstream fout(filename.ToPlatformString().c_str(),
                       std::ios::binary | std::ios::trunc);
    if(!fout)
        return false;
    fout.write(reinterpret_cast<const char*>(data), len);
    fout.close();
    return true;
}

inline bool ReadTextFileRaw(const WStr &filename, WStr *str)
{
    std::wifstream fin(filename.ToPlatformString(), std::ios_base::in);
    if(!fin)
        return false;
    *str = std::wstring(std::istreambuf_iterator<wchar_t>(fin),
                        std::istreambuf_iterator<wchar_t>());
    return true;
}

inline bool WriteTextFileRaw(const WStr &filename, const WStr &str)
{
    std::wofstream fout(filename.ToPlatformString(), std::ios_base::out | std::ios_base::trunc);
    if(!fout)
        return false;
    fout << str.ToStdWString();
    return true;
}

AGZ_NS_END(AGZ::FileSys)
