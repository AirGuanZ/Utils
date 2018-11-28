#pragma once

#include <fstream>

#include "../Utils/Serialize.h"
#include "../Utils/String.h"
#include "File.h"
#include "Path.h"

namespace AGZ::FileSys {

/**
 * @brief 磁盘cache管理器
 */
class BinaryFileCache
{
    template<typename CacheBuilder>
    static auto Build(const Str8 &cacheFilename, CacheBuilder &&cacheBuilder)
    {
        Path8 path(cacheFilename);
        File::CreateDirectoryRecursively(path.ToDirectory().ToStr());

        std::ofstream fout(cacheFilename.ToPlatformString(), std::ios::binary | std::ios::trunc);
        if(!fout)
            throw FileException(("BinaryFileCache: failed to open new cache file: " + Str8(cacheFilename)).ToStdString());
        BinaryOStreamSerializer serializer(fout);
        return cacheBuilder(serializer);
    }

public:

    //! 对给定的文件名，自动构造一个cache文件名
    static Str8 AutoCacheName(const Str8 &filename)
    {
        return Path8("./.agz.cache/").Append(Path8(filename).ToRelative()).ToStr();
    }

    /**
     * @brief 自动cache管理
     * 
     * @param cacheFilename cache文件名
     * 
     * 若cache文件不存在或读取失败，则调用cacheBuilder(serializer)建立cache并返回其结果
     * 否则，调用cacheValidator(deserializer)进行cache的有效性验证，若成功，直接用cacheLoader(deserializer)进行读取并返回结果。
     * 否则，调用cacheBuilder进行cache建立cache并返回结果
     * 
     * cacheBuilder和cacheLoader的返回值类型应该是相同的
     * 
     * @exception FileException cache文件创建或写入失败时抛出
     */
    template<typename CacheBuilder, typename CacheValidator, typename CacheLoader>
    static auto Cache(const Str8 &cacheFilename, CacheBuilder &&builder, CacheValidator &&validator, CacheLoader &&loader)
    {
        {
            std::ifstream fin(cacheFilename.ToPlatformString(), std::ios::binary);
            if(fin)
            {
                BinaryIStreamDeserializer deserializer(fin);
                if(validator(deserializer))
                    return loader(deserializer);
            }
        }
        return Build(cacheFilename, std::forward<CacheBuilder>(builder));
    }
};

}
