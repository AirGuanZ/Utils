#pragma once

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ::Tex)

template<typename PT>
class Texture2D
{
    friend class TextureFile;

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

    AGZ_FORCEINLINE uint32_t Index(uint32_t x, uint32_t y) const
    {
        AGZ_ASSERT(IsAvailable() && x < width_ && y < height_);
        return y * width_ + x;
    }

public:

    using Pixel = PT;
    using Self  = Texture2D<Pixel>;

    Texture2D()
        : width_(0), height_(0), data_(nullptr)
    {
        
    }

    Texture2D(uint32_t w, uint32_t h, const Pixel &initValue = Pixel())
        : Texture2D(w, h, UNINITIALIZED)
    {
        AGZ_ASSERT(w > 0 && h > 0);

        uint32_t cnt = w * h, i = 0;
        try
        {
            for(; i < cnt; ++i)
                new(data_ + i) Pixel(initValue);
        }
        catch(...)
        {
            for(uint32_t j = 0; j < i; ++j)
                (data_ + j)->~PT();
            ::operator delete(data_);
            throw;
        }
    }

    Texture2D(uint32_t w, uint32_t h, Uninitialized_t)
        : width_(w), height_(h)
    {
        AGZ_ASSERT(w > 0 && h > 0);
        data_ = static_cast<Pixel*>(::operator new(sizeof(Pixel) * w * h));
    }

    Texture2D(const Self &copyFrom)
        : width_(copyFrom.width_), height_(copyFrom.height_)
    {
        if(copyFrom.IsAvailable())
        {
            uint32_t cnt = width_ * height_;
            data_ = static_cast<Pixel*>(::operator new(sizeof(Pixel) * cnt));

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

    Texture2D(Self &&moveFrom) noexcept
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

    ~Texture2D()
    {
        if(IsAvailable())
            UncheckedRelease();
    }

    template<typename Func>
    auto Map(Func &&convertFunc)
    {
        AGZ_ASSERT(IsAvailable());

        using RetPixel = decltype(convertFunc(std::declval<Pixel>()));
        Texture2D<RetPixel> ret(width_, height_);

        uint32_t cnt = width_ * height_;
        AGZ_ASSERT(cnt > 0);

        auto dstData = ret.RawData();
        for(uint32_t i = 0; i < cnt; ++i)
            dstData[i] = convertFunc(data_[i]);

        return ret;
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

    void Clear(const Pixel &value)
    {
        AGZ_ASSERT(IsAvailable());
        uint32_t cnt = width_ * height_;
        for(uint32_t i = 0; i < cnt; ++i)
            data_[i] = value;
    }

    Pixel &operator()(uint32_t x, uint32_t y)
    {
        AGZ_ASSERT(IsAvailable());
        return data_[Index(x, y)];
    }

    const Pixel &operator()(uint32_t x, uint32_t y) const
    {
        AGZ_ASSERT(IsAvailable());
        return data_[Index(x, y)];
    }

    uint32_t GetWidth() const
    {
        return width_;
    }

    uint32_t GetHeight() const
    {
        return height_;
    }

    Pixel *RawData()
    {
        return data_;
    }
};

AGZ_NS_END(AGZ::Tex)
