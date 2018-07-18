#pragma once

#include <cstdlib>

#include "../Common.h"

AGZ_NS_BEG(AGZ::Buffer)

template<typename E>
class Buffer2D
{
    size_t d1Size_;
    size_t d2Size_;
    E *data_;

public:

    using Elem = E;
    using Self = Buffer2D<Elem>;

    Buffer2D(size_t d1Size, size_t d2Size, const E &value = E())
        : d1Size_(d1Size), d2Size_(d2Size)
    {
        AGZ_ASSERT(d1Size * d2Size > 0);
        data_ = new E[d1Size * d2Size](value);
    }

    Buffer2D(Self &&other) noexcept
        : d1Size_(other.d1Size_), d2Size_(other.d2Size_), data_(other.data_)
    {
        other.d1Size_ = 0;
        other.d2Size_ = 0;
        other.data_   = nullptr;
    }

    Buffer2D(const Self &other)
        : d1Size_(other.d1Size_), d2Size_(other.d2Size_)
    {
        if(other.data_)
        {
            size_t num = d1Size_ * d2Size_;
            data_ = new E[num];
            for(size_t i = 0; i != num; ++i)
                data_[i] = other.data_[i];
        }
        else
            data_ = nullptr;
    }

    ~Buffer2D()
    {
        if(data_)
            delete data_;
    }

    Self &operator=(const Self &other)
    {
        if(data_)
            delete data_;

        d1Size_ = other.d1Size_;
        d2Size_ = other.d2Size_;

        if(other.data_)
        {
            size_t num = d1Size_ * d2Size_;
            data_ = new E[num];
            for(size_t i = 0; i != num; ++i)
                data_[i] = other.data_[i];
        }
        else
            data_ = nullptr;

        return *this;
    }

    Self &operator=(Self &&other) noexcept
    {
        d1Size_ = other.d1Size_;
        d2Size_ = other.d2Size_;
        data_   = other.data_;

        other.d1Size_ = 0;
        other.d2Size_ = 0;
        other.data_   = nullptr;

        return *this;
    }

    E &operator()(size_t d1Idx, size_t d2Idx)
    {
        return data_[d1Idx * d2Size_ + d2Idx];
    }

    const E &operator()(size_t d1Idx, size_t d2Idx) const
    {
        return data_[d1Idx * d2Size_ + d2Idx];
    }

    size_t GetDem1Size() const
    {
        return d1Size_;
    }

    size_t GetDem2Size() const
    {
        return d2Size_;
    }
};

AGZ_NS_END(AGZ::Buffer)
