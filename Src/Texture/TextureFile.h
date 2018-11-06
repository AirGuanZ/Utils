#pragma once

#include "../Misc/Common.h"
#include "../Misc/Exception.h"
#include "../Utils/Math.h"
#include "../Utils/String.h"
#include "Texture.h"

namespace AGZ {

/**
 * @brief 纹理对象的读取/保存
 */
class TextureFile
{
public:

	//! 可选的保存格式
    enum class WriteFormat
    {
        PNG,
        JPG,
        BMP
    };

	//! 从文件中加载一个二维RGB纹理对象
    static TextureCore<2, Math::Color3b> LoadRGBFromFile(
        const WStr &filename);

	//! 从文件中加载一个二维RGBA纹理对象
    static TextureCore<2, Math::Color4b> LoadRGBAFromFile(
        const WStr &filename);

	//! 将一个二维RGB纹理对象写入到指定格式的文件
    static void WriteTo(
        const WStr &filename,
        const TextureCore<2, Math::Color3b> &tex,
        WriteFormat format);

	//! 将一个二维RGBA纹理对象写入到指定格式的文件
    static void WriteTo(
        const WStr &filename,
        const TextureCore<2, Math::Color4b> &tex,
        WriteFormat format);

	//! 将一个二维RGB纹理对象写入到PNG文件
    static void WriteRGBToPNG(
        const WStr &filename,
        const TextureCore<2, Math::Color3b> &tex);

	//! 将一个二维RGBA纹理对象写入到PNG文件
    static void WriteRGBAToPNG(
        const WStr &filename,
        const TextureCore<2, Math::Color4b> &tex);

	//! 将一个二维RGB纹理对象写入到JPG文件
    static void WriteRGBToJPG(
        const WStr &filename,
        const TextureCore<2, Math::Color3b> &tex);

	//! 将一个二维RGBA纹理对象写入到JPG文件
    static void WriteRGBAToJPG(
        const WStr &filename,
        const TextureCore<2, Math::Color4b> &tex);

	//! 将一个二维RGB纹理对象写入到BMP文件
    static void WriteRGBToBMP(
        const WStr &filename,
        const TextureCore<2, Math::Color3b> &tex);

	//! 将一个二维RGBA纹理对象写入到BMP文件
    static void WriteRGBAToBMP(
        const WStr &filename,
        const TextureCore<2, Math::Color4b> &tex);
};

} // namespace AGZ

#if defined(AGZ_TEXTURE_FILE_IMPL)

namespace AGZ {

#include <vector>

#include "../Utils/FileSys.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#if defined(_MSC_VER)
#define STBI_MSC_SECURE_CRT
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

TextureCore<2, Math::Color3b> TextureFile::LoadRGBFromFile(
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

    TextureCore<2, Math::Color3b> ret({ w, h }, UNINITIALIZED);
    unsigned char *scanlineData = bytes;
    for(int scanline = 0; scanline < h; ++scanline)
    {
        for(int x = 0; x < w; ++x)
        {
            ret({ x, scanline }) = Math::Color3b(scanlineData[0],
                                                 scanlineData[1],
                                                 scanlineData[2]);
            scanlineData += 3;
        }
    }

    stbi_image_free(bytes);
    FileSys::DefaultlyReleaseRawBinaryFileContent(content);

    return ret;
}

TextureCore<2, Math::Color4b> TextureFile::LoadRGBAFromFile(
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

    TextureCore<2, Math::Color4b> ret({ w, h }, UNINITIALIZED);
    unsigned char *scanlineData = bytes;
    for(int scanline = 0; scanline < h; ++scanline)
    {
        for(int x = 0; x < w; ++x)
        {
            ret({ x, scanline }) = Math::Color4b(scanlineData[0],
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

void TextureFile::WriteTo(
    const WStr &filename,
    const TextureCore<2, Math::Color3b> &tex,
    WriteFormat format)
{
    switch(format)
    {
    case WriteFormat::PNG:
        return WriteRGBToPNG(filename, tex);
    case WriteFormat::JPG:
        return WriteRGBToJPG(filename, tex);
    case WriteFormat::BMP:
        return WriteRGBToPNG(filename, tex);
    default:
        Unreachable();
    }
}

void TextureFile::WriteTo(
    const WStr &filename,
    const TextureCore<2, Math::Color4b> &tex,
    WriteFormat format)
{
    switch(format)
    {
    case WriteFormat::PNG:
        return WriteRGBAToPNG(filename, tex);
    case WriteFormat::JPG:
        return WriteRGBAToJPG(filename, tex);
    case WriteFormat::BMP:
        return WriteRGBAToPNG(filename, tex);
    default:
        Unreachable();
    }
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
    const TextureCore<2, Math::Color3b> &tex)
{
    AGZ_ASSERT(tex.IsAvailable());

    std::vector<unsigned char> data;
    BufferContext bc = { &data };
    if(!stbi_write_png_to_func(
            buffer_func, &bc,
            tex.GetSize()[0], tex.GetSize()[1],
            3, tex.RawData(), 0))
    {
        throw FileException("Failed to construct PNG file in memory");
    }

    if(!FileSys::WriteBinaryFileRaw(filename, data.data(), data.size()))
        throw FileException("Failed to write to PNG file");
}

void TextureFile::WriteRGBAToPNG(
    const WStr &filename,
    const TextureCore<2, Math::Color4b> &tex)
{
    AGZ_ASSERT(tex.IsAvailable());

    std::vector<unsigned char> data;
    BufferContext bc = { &data };
    if(!stbi_write_png_to_func(
        buffer_func, &bc,
        tex.GetSize()[0], tex.GetSize()[1],
        4, tex.RawData(), 0))
    {
        throw FileException("Failed to construct PNG file in memory");
    }

    if(!FileSys::WriteBinaryFileRaw(filename, data.data(), data.size()))
        throw FileException("Failed to write to PNG file");
}

void TextureFile::WriteRGBToJPG(
    const WStr &filename,
    const TextureCore<2, Math::Color3b> &tex)
{
    AGZ_ASSERT(tex.IsAvailable());

    std::vector<unsigned char> data;
    BufferContext bc = { &data };
    if(!stbi_write_jpg_to_func(
        buffer_func, &bc,
        tex.GetSize()[0], tex.GetSize()[1],
        3, tex.RawData(), 0))
    {
        throw FileException("Failed to construct JPG file in memory");
    }

    if(!FileSys::WriteBinaryFileRaw(filename, data.data(), data.size()))
        throw FileException("Failed to write to JPG file");
}

void TextureFile::WriteRGBAToJPG(
    const WStr &filename,
    const TextureCore<2, Math::Color4b> &tex)
{
    AGZ_ASSERT(tex.IsAvailable());

    std::vector<unsigned char> data;
    BufferContext bc = { &data };
    if(!stbi_write_jpg_to_func(
        buffer_func, &bc,
        tex.GetSize()[0], tex.GetSize()[1],
        4, tex.RawData(), 0))
    {
        throw FileException("Failed to construct JPG file in memory");
    }

    if(!FileSys::WriteBinaryFileRaw(filename, data.data(), data.size()))
        throw FileException("Failed to write to JPG file");
}

void TextureFile::WriteRGBToBMP(
    const WStr &filename,
    const TextureCore<2, Math::Color3b> &tex)
{
    AGZ_ASSERT(tex.IsAvailable());

    std::vector<unsigned char> data;
    BufferContext bc = { &data };
    if(!stbi_write_bmp_to_func(
        buffer_func, &bc,
        tex.GetSize()[0], tex.GetSize()[1],
        3, tex.RawData()))
    {
        throw FileException("Failed to construct BMP file in memory");
    }

    if(!FileSys::WriteBinaryFileRaw(filename, data.data(), data.size()))
        throw FileException("Failed to write to BMP file");
}

void TextureFile::WriteRGBAToBMP(
    const WStr &filename,
    const TextureCore<2, Math::Color4b> &tex)
{
    AGZ_ASSERT(tex.IsAvailable());

    std::vector<unsigned char> data;
    BufferContext bc = { &data };
    if(!stbi_write_bmp_to_func(
        buffer_func, &bc,
        tex.GetSize()[0], tex.GetSize()[1],
        4, tex.RawData()))
    {
        throw FileException("Failed to construct BMP file in memory");
    }

    if(!FileSys::WriteBinaryFileRaw(filename, data.data(), data.size()))
        throw FileException("Failed to write to BMP file");
}

} // namespace AGZ

#endif
