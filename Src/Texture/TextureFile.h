#pragma once

#include "../Misc/Common.h"
#include "../Utils/Math.h"
#include "Texture2D.h"

AGZ_NS_BEG(AGZ::Tex)

class TextureFile
{
public:

    static Texture2D<Math::Color3b> LoadRGBFromFile(
        const wchar_t *filename);

    static Texture2D<Math::Color4b> LoadRGBAFromFile(
        const wchar_t *filename);
};

AGZ_NS_END(AGZ::Tex)

#ifdef AGZ_TEXTURE_FILE_IMPL

#include "../Utils/FileSys.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

AGZ_NS_BEG(AGZ::Tex)

Texture2D<Math::Color3b> TextureFile::LoadRGBFromFile(
    const wchar_t *filename)
{
    auto [len, content] = FileSys::ReadBinaryFileRaw(filename);
    if(!content)
        throw FileException("Failed to read texture file content");

    int w, h, channels;
    unsigned char *bytes = stbi_load_from_memory(
        content, len, &w, &h, &channels, STBI_rgb);
    if(!bytes)
    {
        FileSys::DefaultlyReleaseRawBinaryFileContent(content);
        throw FileException("Failed to load texture from memory");
    }

    AGZ_ASSERT(w > 0 && h > 0);

    Texture2D<Math::Color3b> ret(w, h, UNINITIALIZED);
    unsigned char *scanlineData = bytes;
    for(int scanline = 0; scanline < h; ++scanline)
    {
        for(int x = 0; x < w; ++x)
        {
            ret(x, scanline) = Math::Color3b(scanlineData[0],
                                             scanlineData[1],
                                             scanlineData[2]);
            scanlineData += 3;
        }
    }

    stbi_image_free(bytes);
    FileSys::DefaultlyReleaseRawBinaryFileContent(content);

    return std::move(ret);
}

Texture2D<Math::Color4b> TextureFile::LoadRGBAFromFile(
    const wchar_t *filename)
{
    auto [len, content] = FileSys::ReadBinaryFileRaw(filename);
    if(!content)
        throw FileException("Failed to read texture file content");

    int w, h, channels;
    unsigned char *bytes = stbi_load_from_memory(
        content, len, &w, &h, &channels, STBI_rgb_alpha);
    if(!bytes)
    {
        FileSys::DefaultlyReleaseRawBinaryFileContent(content);
        throw FileException("Failed to load texture from memory");
    }

    AGZ_ASSERT(w > 0 && h > 0);

    Texture2D<Math::Color4b> ret(w, h, UNINITIALIZED);
    unsigned char *scanlineData = bytes;
    for(int scanline = 0; scanline < h; ++scanline)
    {
        for(int x = 0; x < w; ++x)
        {
            ret(x, scanline) = Math::Color4b(scanlineData[0],
                                             scanlineData[1],
                                             scanlineData[2],
                                             scanlineData[3]);
            scanlineData += 4;
        }
    }

    stbi_image_free(bytes);
    FileSys::DefaultlyReleaseRawBinaryFileContent(content);

    return std::move(ret);
}

AGZ_NS_END(AGZ::Tex)

#endif
