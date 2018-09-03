#pragma once

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

template<typename PT>
class TexData2D
{
    uint32_t width_;
    uint32_t height_;

    PT *data_;

    void UncheckedRelease()
    {
        uint32_t cnt = width_ * height_;
        for(uint32_t i = 0; i < cnt; ++i)
            (data_ + i)->~PT();
        ::operator delete(data_);
    }

public:

    using Pixel = PT;
    using Self  = TexData2D<Pixel>;

    TexData2D()
        : width_(0), height_(0), data_(nullptr)
    {
        
    }

    TexData2D(const Self &copyFrom)
        : width_(copyFrom.width_), height_(copyFrom.height_)
    {
        if(copyFrom.IsAvailable())
        {
            uint32_t cnt = width_ * height_;
            data_ = ::operator new(sizeof(Pixel) * cnt);

            uint32_t i = 0;
            try
            {
                for(; i < cnt; ++i)
                    new(data_ + i) Pixel(copyFrom.data_[i]);
            }
            catch(...)
            {
                for(uint32_t j = 0; j < i; ++j)
                    (data_ + j)->~PT();
                ::operator delete(data_);
                throw;
            }
        }
        else
            data_ = nullptr;
    }

    TexData2D(Self &&moveFrom) noexcept
        : width_(moveFrom.width_), height_(moveFrom.height_),
          data_(moveFrom.data_)
    {
        moveFrom.width_ = moveFrom.height_ = 0;
        moveFrom.data_ = nullptr;
    }

    Self &operator=(const Self &copyFrom)
    {
        if(IsAvailable())
            UncheckedRelease();
        new(this) Self(copyFrom);
        return *this;
    }

    Self &operator=(Self &&moveFrom) noexcept
    {
        if(IsAvailable())
            UncheckedRelease();
        new(this) Self(std::move(moveFrom));
        return *this;
    }

    ~TexData2D()
    {
        if(IsAvailable())
            UncheckedRelease();
    }

    bool IsAvailable() const
    {
        return data_ != nullptr;
    }

    operator bool() const
    {
        return IsAvailable();
    }

    void Destroy()
    {
        if(IsAvailable())
        {
            UncheckedRelease();
            width_ = height_ = 0;
            data_ = nullptr;
        }
    }
};

AGZ_NS_END(AGZ)
