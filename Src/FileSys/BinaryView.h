#pragma once

#include <algorithm>
#include <cstring>
#include <fstream>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

class BinaryStreamCore
{
public:

    virtual ~BinaryStreamCore() = default;

    virtual bool Read(void *data, size_t byteSize) = 0;

    virtual bool Write(const void *data, size_t byteSize) = 0;
};

class BinaryMemoryReadStreamCore : public BinaryStreamCore
{
    const unsigned char *beg_;
    const unsigned char *cur_;
    const unsigned char *end_;

public:

    BinaryMemoryReadStreamCore()
        : beg_(nullptr), cur_(nullptr), end_(nullptr)
    {
        
    }

    BinaryMemoryReadStreamCore(const void *beg, const void *end)
        : beg_(static_cast<const unsigned char*>(beg)),
          cur_(static_cast<const unsigned char*>(beg)),
          end_(static_cast<const unsigned char*>(end))
    {
        AGZ_ASSERT(beg <= end);
    }

    BinaryMemoryReadStreamCore(const void *beg, size_t byteSize)
        : beg_(static_cast<const unsigned char*>(beg)),
          cur_(static_cast<const unsigned char*>(beg)),
          end_(static_cast<const unsigned char*>(beg) + byteSize)
    {
        
    }

    void SetData(const void *beg, const void *end)
    {
        AGZ_ASSERT(beg <= end);
        beg_ = static_cast<const unsigned char*>(beg);
        cur_ = beg_;
        end_ = static_cast<const unsigned char*>(end);
    }

    void SetData(const unsigned char *beg, size_t byteSize)
    {
        beg_ = static_cast<const unsigned char*>(beg);
        cur_ = beg_;
        end_ = beg + byteSize;
    }

    bool Read(void *data, size_t byteSize) override
    {
        if(RemainningSize() >= byteSize)
        {
            std::memcpy(data, cur_, byteSize);
            cur_ += byteSize;
            return true;
        }
        return false;
    }

    bool Write(const void *_data, size_t _byteSize) override
    {
        throw UnreachableException("BinaryMemoryReadStreamCore::Write is uncallable");
    }

    bool IsAvailable() const
    {
        return beg_ != nullptr;
    }

    void Skip(size_t bytes)
    {
        AGZ_ASSERT(IsAvailable());
        cur_ = (std::min)(cur_ + bytes, end_);
    }

    void Seek(size_t pos)
    {
        AGZ_ASSERT(IsAvailable() && pos < Size());
        cur_ = beg_ + pos;
    }

    size_t Tell() const
    {
        AGZ_ASSERT(IsAvailable());
        return cur_ - beg_;
    }

    bool IsEnd() const
    {
        AGZ_ASSERT(IsAvailable());
        return cur_ == end_;
    }

    size_t Size() const
    {
        AGZ_ASSERT(IsAvailable());
        return end_ - beg_;
    }

    size_t RemainningSize() const
    {
        AGZ_ASSERT(IsAvailable());
        return end_ - cur_;
    }
};

class BinaryMemoryWriteStreamCore : public BinaryStreamCore
{
    unsigned char *beg_;
    unsigned char *cur_;
    unsigned char *end_;

public:

    BinaryMemoryWriteStreamCore()
        : beg_(nullptr), cur_(nullptr), end_(nullptr)
    {

    }

    BinaryMemoryWriteStreamCore(void *beg, void *end)
        : beg_(static_cast<unsigned char*>(beg)),
          cur_(static_cast<unsigned char*>(beg)),
          end_(static_cast<unsigned char*>(end))
    {
        AGZ_ASSERT(beg <= end);
    }

    BinaryMemoryWriteStreamCore(void *beg, size_t byteSize)
        : beg_(static_cast<unsigned char*>(beg)),
          cur_(static_cast<unsigned char*>(beg)),
          end_(static_cast<unsigned char*>(beg) + byteSize)
    {

    }

    void SetData(void *beg, void *end)
    {
        AGZ_ASSERT(beg <= end);
        beg_ = static_cast<unsigned char*>(beg);
        cur_ = beg_;
        end_ = static_cast<unsigned char*>(end);
    }

    void SetData(unsigned char *beg, size_t byteSize)
    {
        beg_ = static_cast<unsigned char*>(beg);
        cur_ = beg_;
        end_ = beg + byteSize;
    }

    bool Read(void *data, size_t byteSize) override
    {
        throw UnreachableException("BinaryMemoryWriteStreamCore::Read is uncallable");
    }

    bool Write(const void *data, size_t byteSize) override
    {
        if(RemainningSize() >= byteSize)
        {
            std::memcpy(cur_, data, byteSize);
            cur_ += byteSize;
            return true;
        }
        return false;
    }

    bool IsAvailable() const
    {
        return beg_ != nullptr;
    }

    void Skip(size_t bytes)
    {
        AGZ_ASSERT(IsAvailable());
        cur_ = (std::min)(cur_ + bytes, end_);
    }

    void Seek(size_t pos)
    {
        AGZ_ASSERT(IsAvailable() && pos < Size());
        cur_ = beg_ + pos;
    }

    size_t Tell() const
    {
        AGZ_ASSERT(IsAvailable());
        return cur_ - beg_;
    }

    bool IsEnd() const
    {
        AGZ_ASSERT(IsAvailable());
        return cur_ == end_;
    }

    size_t Size() const
    {
        AGZ_ASSERT(IsAvailable());
        return end_ - beg_;
    }

    size_t RemainningSize() const
    {
        AGZ_ASSERT(IsAvailable());
        return end_ - cur_;
    }
};

class BinaryFileStreamCore : public BinaryStreamCore
{
    std::ifstream *fin_;
    std::ofstream *fout_;

public:

    BinaryFileStreamCore()
        : fin_(nullptr), fout_(nullptr)
    {
        
    }

    explicit BinaryFileStreamCore(std::ifstream &fin)
        : fin_(&fin), fout_(nullptr)
    {
        
    }

    explicit BinaryFileStreamCore(std::ofstream &fout)
        : fin_(nullptr), fout_(&fout)
    {
        
    }

    void SetFile()
    {
        fin_ = nullptr;
        fout_ = nullptr;
    }

    void SetFile(std::ifstream &fin)
    {
        fin_ = &fin;
        fout_ = nullptr;
    }

    void SetFile(std::ofstream &fout)
    {
        fin_ = nullptr;
        fout_ = &fout;
    }

    bool IsReadStream() const
    {
        return fin_ != nullptr;
    }

    bool IsWriteStream() const
    {
        return fout_ != nullptr;
    }

    bool IsAvailable() const
    {
        return IsReadStream() || IsWriteStream();
    }

    bool Read(void *data, size_t byteSize) override
    {
        AGZ_ASSERT(fin_);
        fin_->read(static_cast<char*>(data), byteSize);
        return !!*fin_;
    }

    bool Write(const void *data, size_t byteSize) override
    {
        AGZ_ASSERT(fout_);
        fout_->write(static_cast<const char*>(data), byteSize);
        return !!*fout_;
    }

    void Skip(size_t bytes)
    {
        AGZ_ASSERT(fin_);
        fin_->clear();
        fin_->seekg(static_cast<std::ifstream::pos_type>(bytes),
            std::ios::cur);
    }

    void Seek(size_t pos)
    {
        AGZ_ASSERT(fin_);
        fin_->clear();
        fin_->seekg(static_cast<std::ifstream::pos_type>(pos),
            std::ios::beg);
    }

    size_t Tell() const
    {
        AGZ_ASSERT(fin_);
        fin_->clear();
        return static_cast<size_t>(fin_->tellg());
    }

    bool IsEnd() const
    {
        AGZ_ASSERT(fin_);
        return fin_->eof();
    }

    size_t Size() const
    {
        AGZ_ASSERT(fin_);
        auto oldPos = fin_->tellg();
        fin_->seekg(0, std::ios::end);
        size_t ret = static_cast<size_t>(fin_->tellg());
        fin_->seekg(oldPos, std::ios::beg);
        return ret;
    }
};

template<typename Core>
class BinaryStreamView : public Core
{
public:

    template<typename...Args>
    explicit BinaryStreamView(Args&&...args)
        : Core(std::forward<Args>(args)...)
    {
        
    }

    template<typename T>
    bool Read(T &dst)
    {
        return Read(&dst, sizeof(dst));
    }

    template<typename T>
    bool Write(const T &dst)
    {
        return Write(&dst, sizeof(dst));
    }
};

using BinaryFileStreamView        = BinaryStreamView<BinaryFileStreamCore>;
using BinaryMemoryReadStreamView  = BinaryStreamView<BinaryMemoryReadStreamCore>;
using BinaryMemoryWriteStreamView = BinaryStreamView<BinaryMemoryReadStreamCore>;

AGZ_NS_END(AGZ)
