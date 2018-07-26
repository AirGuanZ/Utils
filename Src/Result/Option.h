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
        new(&data_[0]) Data(copyFrom);
    }

    explicit FixedOption(T &&moveFrom)
        : isSome_(true)
    {
        new(&data_[0]) Data(std::move(moveFrom));
    }

    FixedOption(const Self &copyFrom)
        : isSome_(copyFrom.isSome_)
    {
        if(isSome_)
            new(&data_[0]) Data(*reinterpret_cast<const Data*>(&copyFrom.data_[0]));
    }

    FixedOption(Self &&moveFrom)
        : isSome_(moveFrom.isSome_)
    {
        if(isSome_)
            new(&data_[0]) Data(std::move(*reinterpret_cast<Data*>(&moveFrom.data_[0])));
    }

    ~FixedOption()
    {
        if(isSome_)
            reinterpret_cast<Data*>(&data_[0])->~T();
    }

    Self &operator=(const Self &copyFrom)
    {
        if(isSome_)
            reinterpret_cast<Data*>(&data_[0])->~T();
        if((isSome_ = copyFrom.isSome_))
            new(&data_[0]) Data(*reinterpret_cast<const Data*>(&copyFrom.data_[0]));
        return *this;
    }

    Self &operator=(Self &&moveFrom)
    {
        if(isSome_)
            reinterpret_cast<Data*>(&data_[0])->~T();
        if((isSome_ = moveFrom.isSome_))
            new(&data_[0]) Data(std::move(*reinterpret_cast<Data*>(&moveFrom.data_[0])));
        return *this;
    }

    Type GetType() const { return isSome_ ? Some : None; }
    bool IsSome() const { return isSome_; }
    bool IsNone() const { return !isSome_; }

    T &Unwrap()
    {
        return *reinterpret_cast<Data*>(&data_[0]);
    }

    const T &Unwrap() const
    {
        return *reinterpret_cast<const Data*>(&data_[0]);
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
        data_ = new Data(std::move(moveFrom));
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

    Self &operator=(Self &&moveFrom) noexcept(noexcept(~T()))
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
        return *data_;
    }

    const T &Unwrap() const
    {
        return *data_;
    }
};

namespace Aux
{
    template<typename T>
    std::enable_if_t<sizeof(T) <= 8, FixedOption<T>> OptionSelector() { return FixedOption<T>(); }

    template<typename T>
    std::enable_if_t<(sizeof(T) > 8), AllocOption<T>> OptionSelector() { return AllocOption<T>(); }
}

template<typename T>
using Option = decltype(Aux::OptionSelector<T>());

template<typename T>
Option<T> Some(const T &v) { return Option<T>(v); }

template<typename T>
Option<T> Some(T &&v) { return Option<T>(std::forward<T>(v)); }

template<typename T>
Option<T> None() { return Option<T>(); }

AGZ_NS_END(AGZ)
