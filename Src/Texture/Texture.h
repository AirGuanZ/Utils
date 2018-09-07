#pragma once

#include <type_traits>

#include "../Utils/Math.h"

AGZ_NS_BEG(AGZ::Tex)

template<Math::DimType DIM>
using Coord = Math::Vec<DIM, uint32_t>;

template<Math::DimType DIM, typename PT>
class Texture
{
    friend class TextureFile;

    Math::Vec<DIM, uint32_t> size_;
    uint32_t cnt_;
    PT *data_;

    void UncheckedRelease()
    {
        for(uint32_t i = 0; i < cnt_; ++i)
            (data_ + i)->~PT();
        ::operator delete(data_);
    }

    uint32_t LinearIndex(const Math::Vec<DIM, uint32_t> &coord) const
    {
        AGZ_ASSERT(IsAvailable() && coord.EachElemLessThan(size_));

        if constexpr(DIM == 1)
            return coord[0];
        else if constexpr(DIM == 2)
            return size_[0] * coord[1] + coord[0];
        else if constexpr(DIM == 3)
            return size_[0] * (size_[1] * coord[2] + coord[1]) + coord[0];
        else
        {
            uint32_t ret = coord[DIM - 1];
            for(Math::DimType i = DIM - 1; i > 0; --i)
                ret = size_[i - 1] * ret + coord[i - 1];
            return ret;
        }
    }

public:

    using Pixel   = PT;
    using Self    = Texture<DIM, PT>;
    using Coord   = Math::Vec<DIM, uint32_t>;
    using DimType = Math::DimType;

    static constexpr DimType Dim = DIM;

    Texture()
        : size_(0), cnt_(0), data_(nullptr)
    {
        
    }

    explicit Texture(const Coord &size, const Pixel &initValue = Pixel())
        : Texture(size, UNINITIALIZED)
    {
        AGZ_ASSERT(Coord(0).EachElemLessThan(size));
        ConstructN<Pixel, OperatorDeleter>(data_, cnt_, initValue);
    }

    Texture(const Coord &size, Uninitialized_t)
        : size_(size), cnt_(size.Product())
    {
        AGZ_ASSERT(Coord(0).EachElemLessThan(size));
        data_ = static_cast<Pixel*>(::operator new(sizeof(Pixel) * cnt_));
    }

    Texture(const Self &copyFrom)
        : size_(copyFrom.size_), cnt_(copyFrom.cnt_)
    {
        if(copyFrom.IsAvailable())
        {
            data_ = static_cast<Pixel*>(::operator new(sizeof(Pixel) * cnt_));

            uint32_t i = 0;
            try
            {
                for(; i < cnt_; ++i)
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

    Texture(Self &&moveFrom) noexcept
        : size_(moveFrom.size_), cnt_(moveFrom.cnt_), data_(moveFrom.data_)
    {
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

    ~Texture()
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
            data_ = nullptr;
        }
    }

    void Clear(const Pixel &value)
    {
        AGZ_ASSERT(IsAvailable());
        for(uint32_t i = 0; i < cnt_; ++i)
            data_[i] = value;
    }

    const Pixel &operator()(const Coord &coord) const
    {
        AGZ_ASSERT(IsAvailable());
        return data_[LinearIndex(coord)];
    }

    Pixel &operator()(const Coord &coord)
    {
        AGZ_ASSERT(IsAvailable());
        return data_[LinearIndex(coord)];
    }

    const Pixel &operator()(uint32_t x) const
    {
        AGZ_ASSERT(IsAvailable());
        static_assert(Dim == 1);
        return data_[x];
    }

    Pixel &operator()(uint32_t x)
    {
        AGZ_ASSERT(IsAvailable());
        static_assert(Dim == 1);
        return data_[x];
    }

    const Pixel &operator()(uint32_t x, uint32_t y) const
    {
        AGZ_ASSERT(IsAvailable());
        static_assert(Dim == 2);
        return data_[LinearIndex({ x, y })];
    }

    Pixel &operator()(uint32_t x, uint32_t y)
    {
        AGZ_ASSERT(IsAvailable());
        static_assert(Dim == 2);
        return data_[LinearIndex({ x, y })];
    }

    const Pixel &At(const Coord &coord) const
    {
        return (*this)(coord);
    }

    Pixel &At(const Coord &coord)
    {
        return (*this)(coord);
    }

    const Pixel &At(uint32_t x) const
    {
        AGZ_ASSERT(IsAvailable());
        static_assert(Dim == 1);
        return data_[x];
    }

    Pixel &At(uint32_t x)
    {
        AGZ_ASSERT(IsAvailable());
        static_assert(Dim == 1);
        return data_[x];
    }

    const Pixel &At(uint32_t x, uint32_t y) const
    {
        AGZ_ASSERT(IsAvailable());
        static_assert(Dim == 2);
        return data_[LinearIndex({ x, y })];
    }

    Pixel &At(uint32_t x, uint32_t y)
    {
        AGZ_ASSERT(IsAvailable());
        static_assert(Dim == 2);
        return data_[LinearIndex({ x, y })];
    }

    const Coord &GetSize() const
    {
        AGZ_ASSERT(IsAvailable());
        return size_;
    }

    uint32_t GetWidth() const
    {
        static_assert(Dim == 2);
        return size_[0];
    }

    uint32_t GetHeight() const
    {
        static_assert(Dim == 2);
        return size_[1];
    }

    Pixel *RawData()
    {
        AGZ_ASSERT(IsAvailable());
        return data_;
    }

    const Pixel *RawData() const
    {
        AGZ_ASSERT(IsAvailable());
        return data_;
    }
};

AGZ_NS_END(AGZ::Tex)
