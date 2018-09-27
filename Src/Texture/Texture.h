#pragma once

#include <type_traits>

#include "../Utils/Math.h"

AGZ_NS_BEG(AGZ::Tex)

template<Math::DimType DIM>
using Coord = Math::Vec<DIM, uint32_t>;

template<Math::DimType DIM, typename PT>
class TextureCore
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
    using Self    = TextureCore<DIM, PT>;
    using Coord   = Math::Vec<DIM, uint32_t>;
    using DimType = Math::DimType;

    static constexpr DimType Dim = DIM;

    TextureCore()
        : size_(0), cnt_(0), data_(nullptr)
    {
        
    }

    explicit TextureCore(const Coord &size, const Pixel &initValue = Pixel())
        : TextureCore(size, UNINITIALIZED)
    {
        AGZ_ASSERT(Coord(0).EachElemLessThan(size));
        ConstructN<Pixel, OperatorDeleter>(data_, cnt_, initValue);
    }

    TextureCore(const Coord &size, Uninitialized_t)
        : size_(size), cnt_(size.Product())
    {
        AGZ_ASSERT(Coord(0).EachElemLessThan(size));
        data_ = static_cast<Pixel*>(::operator new(sizeof(Pixel) * cnt_));
    }

    TextureCore(const Self &copyFrom)
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

    TextureCore(Self &&moveFrom) noexcept
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

    ~TextureCore()
    {
        if(IsAvailable())
            UncheckedRelease();
    }

    Self &GetCore()
    {
        return *this;
    }

    const Self &GetCore() const
    {
        return *this;
    }

    bool IsAvailable() const
    {
        return data_ != nullptr;
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

    const Pixel &At(const Coord &coord) const
    {
        return (*this)(coord);
    }

    Pixel &At(const Coord &coord)
    {
        return (*this)(coord);
    }

    const Coord &GetSize() const
    {
        AGZ_ASSERT(IsAvailable());
        return size_;
    }

    uint32_t GetLinearSize() const
    {
        return cnt_;
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

    template<typename F>
    auto Map(F &&func) const
    {
        AGZ_ASSERT(IsAvailable());

        using NewPixel = decltype(func(std::declval<Pixel>()));
        TextureCore<Dim, NewPixel> ret(size_);
        NewPixel *newData = ret.RawData();

        for(uint32_t i = 0; i < cnt_; ++i)
            newData[i] = func(data_[i]);
        
        return ret;
    }
};

template<typename PT>
class Texture1D : public TextureCore<1, PT>
{
public:

    using Core = TextureCore<1, PT>;

    explicit Texture1D(TextureCore<1, PT> &&moveFrom) noexcept
        : Core(std::move(moveFrom))
    {

    }

    using Pixel = PT;
    using Self = Texture1D<PT>;
    using Coord = Math::Vec<1, uint32_t>;
    using DimType = Math::DimType;

    Texture1D() = default;

    explicit Texture1D(uint32_t size, const Pixel &initVal = Pixel())
        : Core(Coord(size), initVal)
    {

    }

    Texture1D(const Self &)       = default;
    Self &operator=(const Self &) = default;

    Texture1D(Self &&moveFrom) noexcept
        : Core(std::move(moveFrom.GetCore()))
    {

    }

    Self &operator=(Self &&moveFrom) noexcept
    {
        GetCore() = std::move(moveFrom.GetCore());
        return *this;
    }

    uint32_t GetLength() const
    {
        return Core::GetSize()[0];
    }

    const Pixel &operator()(uint32_t idx) const
    {
        return Core::At(Coord(idx));
    }

    Pixel &operator()(uint32_t idx)
    {
        return Core::At(Coord(idx));
    }

    const Pixel &At(uint32_t idx) const
    {
        return Core::At(Coord(idx));
    }

    Pixel &At(uint32_t idx)
    {
        return Core::At(Coord(idx));
    }

    template<typename F>
    auto Map(F &&func) const
    {
        return Self(Core::Map(std::forward<F>(func)));
    }
};

template<typename PT>
class Texture2D : public TextureCore<2, PT>
{
public:

    using Core = TextureCore<2, PT>;

    explicit Texture2D(TextureCore<2, PT> &&moveFrom) noexcept
        : Core(std::move(moveFrom))
    {
        
    }

    using Pixel   = PT;
    using Self    = Texture2D<PT>;
    using Coord   = Math::Vec<2, uint32_t>;
    using DimType = Math::DimType;
    
    Texture2D() = default;
    
    Texture2D(uint32_t w, uint32_t h, const Pixel &initVal = Pixel())
        : Core(Coord(w, h), initVal)
    {
        
    }

    Texture2D(const Self &)       = default;
    Self &operator=(const Self &) = default;
    
    Texture2D(Self &&moveFrom) noexcept
        : Core(std::move(moveFrom.GetCore()))
    {
        
    }

    Self &operator=(Self &&moveFrom) noexcept
    {
        GetCore() = std::move(moveFrom.GetCore());
        return *this;
    }

    uint32_t GetWidth() const
    {
        return Core::GetSize()[0];
    }

    uint32_t GetHeight() const
    {
        return Core::GetSize()[1];
    }

    const Pixel &operator()(uint32_t x, uint32_t y) const
    {
        return Core::At(Coord(x, y));
    }

    Pixel &operator()(uint32_t x, uint32_t y)
    {
        return Core::At(Coord(x, y));
    }

    const Pixel &At(uint32_t x, uint32_t y) const
    {
        return Core::At(Coord(x, y));
    }

    Pixel &At(uint32_t x, uint32_t y)
    {
        return Core::At(Coord(x, y));
    }

    template<typename F>
    auto Map(F &&func) const
    {
        return Self(Core::Map(std::forward<F>(func)));
    }
};


template<typename PT>
class Texture3D : public TextureCore<3, PT>
{
public:

    using Core = TextureCore<3, PT>;

    explicit Texture3D(TextureCore<3, PT> &&moveFrom) noexcept
        : Core(std::move(moveFrom))
    {

    }

    using Pixel   = PT;
    using Self    = Texture3D<PT>;
    using Coord   = Math::Vec<3, uint32_t>;
    using DimType = Math::DimType;

    Texture3D() = default;

    Texture3D(uint32_t x, uint32_t y, uint32_t z, const Pixel &initVal = Pixel())
        : Core(Coord(x, y, z), initVal)
    {

    }

    Texture3D(const Self &)       = default;
    Self &operator=(const Self &) = default;

    Texture3D(Self &&moveFrom) noexcept
        : Core(std::move(moveFrom.GetCore()))
    {

    }

    Self &operator=(Self &&moveFrom) noexcept
    {
        GetCore() = std::move(moveFrom.GetCore());
        return *this;
    }

    uint32_t GetXSize() const
    {
        return Core::GetSize()[0];
    }

    uint32_t GetYSize() const
    {
        return Core::GetSize()[1];
    }

    uint32_t GetZSize() const
    {
        return Core::GetSize()[2];
    }

    const Pixel &operator()(uint32_t x, uint32_t y, uint32_t z) const
    {
        return Core::At(Coord(x, y, z));
    }

    Pixel &operator()(uint32_t x, uint32_t y, uint32_t z)
    {
        return Core::At(Coord(x, y, z));
    }

    const Pixel &At(uint32_t x, uint32_t y, uint32_t z) const
    {
        return Core::At(Coord(x, y, z));
    }

    Pixel &At(uint32_t x, uint32_t y, uint32_t z)
    {
        return Core::At(Coord(x, y, z));
    }

    template<typename F>
    auto Map(F &&func) const
    {
        return Self(Core::Map(std::forward<F>(func)));
    }
};

template<template<typename> typename E, typename EE, Math::DimType DIM,
            std::enable_if_t<std::is_floating_point_v<EE>, int> = 0>
auto ClampedF2B(const TextureCore<DIM, E<EE>> &tex)
{
    AGZ_ASSERT(tex.IsAvailable());
    TextureCore<DIM, E<uint8_t>> ret(tex.GetSize());
    const E<EE> *texRawData = tex.RawData();
    E<uint8_t> *retRawData = ret.RawData();
    uint32_t size = ret.GetLinearSize();

    for(uint32_t i = 0; i < size; ++i)
    {
        retRawData[i] = texRawData[i].Map<uint8_t>([](EE v)
        { return static_cast<uint8_t>(Math::Clamp(v, EE(0), EE(1)) * 255); });
    }

    return ret;
}

template<template<typename> typename E, typename EE,
    std::enable_if_t<std::is_floating_point_v<EE>, int> = 0>
    auto ClampedF2B(const Texture1D<E<EE>> &tex)
{
    return Texture1D<E<uint8_t>>(ClampedF2B(tex.GetCore()));
}

template<template<typename> typename E, typename EE,
    std::enable_if_t<std::is_floating_point_v<EE>, int> = 0>
auto ClampedF2B(const Texture2D<E<EE>> &tex)
{
    return Texture2D<E<uint8_t>>(ClampedF2B(tex.GetCore()));
}

template<template<typename> typename E, typename EE,
    std::enable_if_t<std::is_floating_point_v<EE>, int> = 0>
    auto ClampedF2B(const Texture3D<E<EE>> &tex)
{
    return Texture3D<E<uint8_t>>(ClampedF2B(tex.GetCore()));
}

AGZ_NS_END(AGZ::Tex)
