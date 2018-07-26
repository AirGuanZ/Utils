#pragma once

#include <type_traits>

#include "../Common.h"
#include "Option.h"

AGZ_NS_BEG(AGZ)

enum ResultType { RT_Ok, RT_Err };

template<typename T, typename F>
class alignas(alignof(T), alignof(F)) FixedResult
{
    static const size_t DATA_SIZE = sizeof(T) > sizeof(F) ?
                                    sizeof(T) : sizeof(F);

    struct OK_t  { };
    struct ERR_t { };

    unsigned char data_[DATA_SIZE];
    bool isOk_;

    FixedResult(OK_t, const T &v)
        : isOk_(true)
    {
        new(data_) T(v);
    }

    FixedResult(OK_t, T &&v)
        : isOk_(true)
    {
        new(data_) T(std::move(v));
    }

    FixedResult(ERR_t, const F &v)
        : isOk_(false)
    {
        new(data_) F(v);
    }

    FixedResult(ERR_t, F &&v)
        : isOk_(false)
    {
        new(data_) F(std::move(v));
    }

public:

    using Type = ResultType;
    using OkData  = T;
    using ErrData = F;

    using Self    = FixedResult<T, F>;

    static Self MakeOk(const T &v) { return Self(OK_t(), v); }
    static Self MakeOk(T &&v) { return Self(OK_t(), std::move(v)); }

    static Self MakeErr(const F &v) { return Self(ERR_t(), v); }
    static Self MakeErr(F &&v) { return Self(ERR_t(), std::move(v)); }

    FixedResult(const Self &copyFrom)
        : isOk_(copyFrom.isOk_)
    {
        if(isOk_)
            new(&data_[0]) T(*reinterpret_cast<const T*>(&copyFrom.data_[0]));
        else
            new(&data_[0]) F(*reinterpret_cast<const F*>(&copyFrom.data_[0]));
    }

    FixedResult(Self &&copyFrom)
        : isOk_(copyFrom.isOk_)
    {
        if(isOk_)
            new(&data_[0]) T(std::move(*reinterpret_cast<T*>(
                                            &copyFrom.data_[0])));
        else
            new(&data_[0]) F(std::move(*reinterpret_cast<F*>(
                                            &copyFrom.data_[0])));
    }

    ~FixedResult()
    {
        if(isOk_)
            reinterpret_cast<T*>(&data_[0])->~T();
        else
            reinterpret_cast<F*>(&data_[0])->~F();
    }

    Self &operator=(const Self &copyFrom)
    {
        isOk_ = copyFrom.isOk_;
        if(isOk_)
            new(&data_[0]) T(*reinterpret_cast<const T*>(&copyFrom.data_[0]));
        else
            new(&data_[0]) F(*reinterpret_cast<const F*>(&copyFrom.data_[0]));
        return *this;
    }

    Self &operator=(Self &&moveFrom)
    {
        isOk_ = moveFrom.isOk_;
        if(isOk_)
            new(&data_[0]) T(std::move(*reinterpret_cast<const T*>(
                                            &moveFrom.data_[0])));
        else
            new(&data_[0]) F(std::move(*reinterpret_cast<const F*>(
                                            &moveFrom.data_[0])));
        return *this;
    }

    Type GetType() const { return isOk_ ? RT_Ok : RT_Err; }
    bool IsOk() const { return isOk_; }
    bool IsErr() const { return !isOk_; }

    T &UnwrapOk()
    {
        return *reinterpret_cast<T*>(&data_[0]);
    }

    const T &UnwrapOk() const
    {
        return *reinterpret_cast<T*>(&data_[0]);
    }

    F &UnwrapErr()
    {
        return *reinterpret_cast<F*>(&data_[0]);
    }

    const F &UnwrapErr() const
    {
        return *reinterpret_cast<F*>(&data_[0]);
    }
};

template<typename T, typename F>
class AllocResult
{
    Option<T> ok_;
    Option<T> err_;

    struct OK_t { };
    struct ERR_t { };

    AllocResult(OK_t, const T &v) : ok_(Some(v)), err_() { }
    AllocResult(OK_t, T &&v) : ok_(Some(std::move(v))), err_() { }
    AllocResult(ERR_t, const T &v) : ok_(), err_(Some(v)) { }
    AllocResult(ERR_t, T &&v) : ok_(), err_(Some(std::move(v))) { }

public:

    using Type = ResultType;
    using OkData  = T;
    using ErrData = F;

    using Self = AllocResult<T, F>;

    static Self MakeOk(const T &v) { return Self(OK_t(), v); }
    static Self MakeOk(T &&v) { return Self(OK_t(), std::move(v)); }

    static Self MakeErr(const F &v) { return Self(ERR_t(), v); }
    static Self MakeErr(F &&v) { return Self(ERR_t(), std::move(v)); }

    AllocResult(const Self &copyFrom)
        : ok_(copyFrom.ok_), err_(copyFrom.err_)
    {

    }

    AllocResult(Self &&moveFrom)
        : ok_(std::move(moveFrom.ok_)), err_(std::move(moveFrom.err_))
    {

    }

    ~AllocResult() { }

    Self &operator=(const Self &copyFrom)
    {
        ok_ = copyFrom.ok_;
        err_ = copyFrom.err_;
        return *this;
    }

    Self &operator=(Self &&moveFrom)
    {
        ok_ = std::move(moveFrom.ok_);
        err_ = std::move(moveFrom.err_);
        return *this;
    }

    Type GetType() const { return ok_.IsSome() ? RT_Ok : RT_Err; }
    bool IsOk() const { return ok_.IsSome(); }
    bool IsErr() const { return err_.IsSome(); }

    T &UnwrapOk() { return ok_.Unwrap(); }
    const T &UnwrapOk() const { return ok_.Unwrap(); }

    F &UnwrapErr() { return err_.Unwrap(); }
    const F &UnwrapErr() const { return err_.Unwrap(); }
};

namespace Aux
{
    template<typename T, typename F,
        std::enable_if_t<(sizeof(T) <= 8 && sizeof(F) <= 8), int> N = 0>
    FixedResult<T, F> *ResultSelector() { return nullptr; }

    template<typename T, typename F,
        std::enable_if_t<(sizeof(T) > 8 || sizeof(F) > 8), int> N = 0>
    AllocResult<T, F> *ResultSelector() { return nullptr; }
}

template<typename T, typename F>
using Result = std::remove_pointer_t<decltype(Aux::ResultSelector<T, F>())>;

template<typename T, typename F>
Result<T, F> Ok(const T &v) { return Result<T, F>::MakeOk(v); }

template<typename T, typename F>
Result<T, F> Ok(T &&v) { return Result<T, F>::MakeOk(std::move(v)); }

template<typename T, typename F>
Result<T, F> Err(const F &v) { return Result<T, F>::MakeErr(v); }

template<typename T, typename F>
Result<T, F> Err(F &&v) { return Result<T, F>::MakeErr(std::move(v)); }

AGZ_NS_END(AGZ)
