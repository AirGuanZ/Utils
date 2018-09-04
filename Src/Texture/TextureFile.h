#pragma once

#include "../Misc/Common.h"
#include "../Utils/Math.h"
#include "../Utils/String.h"
#include "Texture2D.h"

AGZ_NS_BEG(AGZ::Tex)

class TextureFile
{
public:

    static Texture2D<Math::Color3b> LoadRGBFromFile(
        const WStr &filename);

    static Texture2D<Math::Color4b> LoadRGBAFromFile(
        const WStr &filename);

    static void WriteRGBToPNG(
        const WStr &filename,
        const Texture2D<Math::Color3b> &tex);

    static void WriteRGBAToPNG(
        const WStr &filename,
        const Texture2D<Math::Color4b> &tex);
};

AGZ_NS_END(AGZ::Tex)

#if defined(AGZ_TEXTURE_FILE_IMPL)

#include <vector>

#include "../Utils/FileSys.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#if defined(_MSC_VER)
#define STBI_MSC_SECURE_CRT
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

AGZ_NS_BEG(AGZ::Tex)

Texture2D<Math::Color3b> TextureFile::LoadRGBFromFile(
    const WStr &filename)
{
    auto [len, content] = FileSys::ReadBinaryFileRaw(filename);
    if(!content)
        throw FileException("Failed to read texture file content");

    int w, h, channels;
    unsigned char *bytes = stbi_load_from_memory(
        content, static_cast<int>(len), &w, &h, &channels, STBI_rgb);
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

    return ret;
}

Texture2D<Math::Color4b> TextureFile::LoadRGBAFromFile(
    const WStr &filename)
{
    auto [len, content] = FileSys::ReadBinaryFileRaw(filename);
    if(!content)
        throw FileException("Failed to read texture file content");

    int w, h, channels;
    unsigned char *bytes = stbi_load_from_memory(
        content, static_cast<int>(len), &w, &h, &channels, STBI_rgb_alpha);
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

    return ret;
}

namespace
{
    struct BufferContext
    {
        std::vector<unsigned char> *data;
    };

    void buffer_func(void *context, void *data, int size)
    {
        auto bc = reinterpret_cast<BufferContext*>(context);
        AGZ_ASSERT(bc && bc->data);
        size_t oldSize = bc->data->size();
        bc->data->resize(oldSize + size);
        std::memcpy(bc->data->data() + oldSize, data, size);
    }
}

void TextureFile::WriteRGBToPNG(
    const WStr &filename,
    const Texture2D<Math::Color3b> &tex)
{
    AGZ_ASSERT(tex);

    std::vector<unsigned char> data;
    BufferContext bc = { &data };
    if(!stbi_write_png_to_func(
            buffer_func, &bc,
            tex.GetWidth(), tex.GetHeight(),
            3, &tex(0, 0), 0))
    {
        throw FileException("Failed to construct PNG file in memory");
    }

    if(!FileSys::WriteBinaryFileRaw(filename, data.data(), data.size()))
        throw FileException("Failed to write to PNG file");
}

void TextureFile::WriteRGBAToPNG(
    const WStr &filename,
    const Texture2D<Math::Color4b> &tex)
{
    AGZ_ASSERT(tex);

    std::vector<unsigned char> data;
    BufferContext bc = { &data };
    if(!stbi_write_png_to_func(
        buffer_func, &bc,
        tex.GetWidth(), tex.GetHeight(),
        4, &tex(0, 0), 0))
    {
        throw FileException("Failed to construct PNG file in memory");
    }

    if(!FileSys::WriteBinaryFileRaw(filename, data.data(), data.size()))
        throw FileException("Failed to write to PNG file");
}

AGZ_NS_END(AGZ::Tex)

#endif
