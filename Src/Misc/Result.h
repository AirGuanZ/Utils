#pragma once

#include <optional>
#include <type_traits>

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

enum ResultType { RT_Ok, RT_Err };

template<typename T, typename F>
class Result
{
    std::optional<T> ok_;
    std::optional<T> err_;

    struct OK_t { };
    struct ERR_t { };

    Result(OK_t, const T &v) : ok_(v), err_() { }
    Result(OK_t, T &&v) : ok_(std::move(v)), err_() { }
    Result(ERR_t, const T &v) : ok_(), err_(v) { }
    Result(ERR_t, T &&v) : ok_(), err_(std::move(v)) { }

public:

    using Type = ResultType;
    using OkData  = T;
    using ErrData = F;

    using Self = Result<T, F>;

    static Self MakeOk(const T &v) { return Self(OK_t(), v); }
    static Self MakeOk(T &&v) { return Self(OK_t(), std::move(v)); }

    static Self MakeErr(const F &v) { return Self(ERR_t(), v); }
    static Self MakeErr(F &&v) { return Self(ERR_t(), std::move(v)); }

    Result(const Self &copyFrom)
        : ok_(copyFrom.ok_), err_(copyFrom.err_)
    {

    }

    Result(Self &&moveFrom)
        : ok_(std::move(moveFrom.ok_)), err_(std::move(moveFrom.err_))
    {

    }

    ~Result() { }

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

    Type GetType() const { return ok_.has_value() ? RT_Ok : RT_Err; }
    bool IsOk() const { return ok_.has_value(); }
    bool IsErr() const { return err_.has_value(); }

    T &UnwrapOk() { return ok_.value(); }
    const T &UnwrapOk() const { return ok_.value(); }

    F &UnwrapErr() { return err_.value(); }
    const F &UnwrapErr() const { return err_.value(); }
};

template<typename T, typename F>
Result<T, F> Ok(const T &v) { return Result<T, F>::MakeOk(v); }

template<typename T, typename F>
Result<T, F> Ok(T &&v) { return Result<T, F>::MakeOk(std::move(v)); }

template<typename T, typename F>
Result<T, F> Err(const F &v) { return Result<T, F>::MakeErr(v); }

template<typename T, typename F>
Result<T, F> Err(F &&v) { return Result<T, F>::MakeErr(std::move(v)); }

AGZ_NS_END(AGZ)
