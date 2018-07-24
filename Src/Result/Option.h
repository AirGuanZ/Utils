#pragma once

#include <exception>
#include <type_traits>
#include <utility>

#include "../Common.h"

AGZ_NS_BEG(AGZ)

template<typename T>
class alignas(alignof(T)) FixedOption
{
    unsigned char data_[sizeof(T)];
    bool isSome_;

public:

    enum Type { Some, None };
    using Data = T;
    using Self = FixedOption<T>;

    FixedOption() : isSome_(false) { }

    explicit FixedOption(const T &copyFrom)
        : isSome_(true)
    {
        new(data_) Data(copyFrom);
    }

    explicit FixedOption(T &&moveFrom)
        : isSome_(true)
    {
        new(data_) Data(std::move(moveFrom));
    }

    FixedOption(const Self &copyFrom)
        : isSome_(copyFrom.isSome_)
    {
        if(isSome_)
            new(data_) Data(*static_cast<const Data*>(copyFrom.data_));
    }

    FixedOption(Self &&moveFrom)
        : isSome_(copyFrom.isSome_)
    {
        if(isSome_)
            new(data_) Data(std::move(*static_cast<Data*>(copyFrom.data_)));
    }

    ~FixedOption()
    {
        if(isSome_)
            *static_cast<Data*>(data_).~T();
    }

    Self &operator=(const Self &copyFrom)
    {
        if(isSome_)
            *static_cast<Data*>(data_).~T();
        if((isSome_ = copyFrom.isSome_))
            new(data_) Data(*static_cast<const Data*>(copyFrom.data_));
        return *this;
    }

    Self &operator=(Self &&moveFrom)
    {
        if(isSome_)
            *static_cast<Data*>(data_).~T();
        if((isSome_ = moveFrom.isSome_))
            new(data_) Data(std::move(*static_cast<Data*>(copyFrom.data_)));
        return *this;
    }

    Type GetType() const { return isSome_ ? Some : None; }
    bool IsSome() const { return isSome_; }
    bool IsNone() const { return !isSome_; }

    T &Unwrap()
    {
        if(!isSome_)
            std::terminate();
        return *static_cast<Data*>(data_);
    }

    const T &Unwrap() const
    {
        if(!isSome_)
            std::terminate();
        return *static_cast<const Data*>(data_);
    }
};

template<typename T>
class AllocOption
{
    T *data_;

public:

    enum Type { Some, None };
    using Data = T;
    using Self = AllocOption<T>;
    
    AllocOption() : data_(nullptr) { }
    
    explicit AllocOption(const T &copyFrom)
    {
        data_ = new Data(copyFrom);
    }
    
    explicit AllocOption(T &&moveFrom)
    {
        data_ = new Data(std::move(copyFrom));
    }
    
    AllocOption(const Self &copyFrom)
    {
        if(copyFrom.IsSome())
            data_ = new Data(*static_cast<const Data*>(copyFrom.data_));
        else
            data_ = nullptr;
    }
    
    AllocOption(Self &&moveFrom)
        : data_(moveFrom.data_)
    {
        moveFrom.data_ = nullptr;
    }
    
    ~AllocOption()
    {
        if(data_)
            delete data_;
    }
    
    Self &operator=(const Self &copyFrom)
    {
        if(data_)
            delete data_;
        if(copyFrom.IsSome())
            data_ = new Data(*static_cast<const Data*>(copyFrom.data_));
        else
            data_ = nullptr;
        return *this;
    }
    
    Self &operator=(Self &&moveFrom)
    {
        if(data_)
            delete data_;
        data_ = moveFrom.data_;
        moveFrom.data_ = nullptr;
        return *this;
    }
    
    Type GetType() const { return data_ ? Some : None; }
    bool IsSome() const { return data_ != nullptr; }
    bool IsNone() const { return data_ == nullptr; }
    
    T &Unwrap()
    {
        if(!data_)
            std::terminate();
        return *data_;
    }
    
    const T &Unwrap()
    {
        if(!data_)
            std::terminate();
        return *data_;
    }
};

namespace Aux
{
    template<typename T>
    std::enable_if_t<sizeof(T) <= 16, FixedOption<T>> OptionSelector() { }
    
    template<typename T>
    std::enable_if_t<sizeof(T) <= 16, AllocOption<T>> OptionSelector() { }
}

template<typename T>
using Option = decltype(Aux::OptionSelector<T>());

AGZ_NS_END(AGZ)
