#pragma once

#include <fstream>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

class BinaryFileView
{
    std::ifstream *fin_;
    std::ofstream *fout_;

public:

    BinaryFileView()
        : fin_(nullptr), fout_(nullptr)
    {

    }

    explicit BinaryFileView(std::ifstream &fin)
        : BinaryFileView()
    {
        SetFile(fin);
    }

    explicit BinaryFileView(std::ofstream &fout)
        : BinaryFileView()
    {
        SetFile(fout);
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

    bool Read(void *data, size_t byteSize)
    {
        AGZ_ASSERT(fin_);
        fin_->read(static_cast<char*>(data), byteSize);
        return !!*fin_;
    }

    bool Write(const void *data, size_t byteSize)
    {
        AGZ_ASSERT(fout_);
        fout_->write(static_cast<const char*>(data), byteSize);
        return !!*fout_;
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

    bool IsEOF() const
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

AGZ_NS_END(AGZ)
