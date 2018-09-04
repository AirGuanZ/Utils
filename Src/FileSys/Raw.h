#pragma once

#include <fstream>

#include "../Misc/Common.h"
#include "../Utils/String.h"

AGZ_NS_BEG(AGZ::FileSys)

template<typename AllocFunc = void*(*)(size_t)>
std::pair<size_t, unsigned char*> ReadBinaryFileRaw(
    const wchar_t *filename, AllocFunc &&func = &(std::malloc))
{
    std::ifstream fin(filename, std::ios::in | std::ios::binary);
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
    const wchar_t *filename,
    const unsigned char *data, size_t len)
{
    std::ofstream fout(filename, std::ios::binary | std::ios::trunc);
    if(!fout)
        return false;
    fout.write(reinterpret_cast<const char*>(data), len);
    fout.close();
    return true;
}

AGZ_NS_END(AGZ::FileSys)
