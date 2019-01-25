#pragma once

#include <vector>

#include "../Misc/Common.h"

namespace AGZ
{

class Accumulator : public Uncopiable
{
    std::vector<char> rowBuf_;
    size_t curSize_;
    char *curTop_;

public:

    explicit Accumulator(size_t initBufSize = 128)
        : rowBuf_(initBufSize), curSize_(initBufSize)
    {
        curTop_ = rowBuf_.data();
    }

    template<typename T>
    auto &Add(T &&value)
    {
        using OT = remove_rcv_t<T>;

        if(curSize_ + sizeof(OT) > rowBuf_.size())
        {
            rowBuf_.resize(2 * rowBuf_.size());
            curTop_ = rowBuf_.data() + curSize_;
        }

        auto pRet = reinterpret_cast<OT*>(curTop_);
        *pRet = std::forward<T>(value);

        curSize_ += sizeof(OT);
        curTop_ += sizeof(OT);
        AGZ_ASSERT(curSize_ <= rowBuf_.size());
        AGZ_ASSERT(curTop_ == rowBuf_.data() + curSize_);

        return *pRet;
    }

    const char *GetBuffer() const noexcept
    {
        return rowBuf_.data();
    }
};

class Fetcher
{
    const char *buffer_;
    const char *pc_;

public:

    explicit Fetcher(const char *buffer) noexcept
        : buffer_(buffer), pc_(buffer)
    {

    }

    void ResetPC()
    {
        AGZ_ASSERT(buffer_);
        pc_ = buffer_;
    }

    template<typename TType>
    auto &FetchNextCMDParam()
    {
        auto *pParam = reinterpret_cast<const TType*>(pc_);
        pc_ = reinterpret_cast<const char*>(pParam + 1);
        return *pParam;
    }
};

} // namespace AGZ
