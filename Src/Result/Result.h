#pragma once

#include "../Common.h"

AGZ_NS_BEG(AGZ)

namespace Aux
{
    template<typename T>
    constexpr T StaticMax(T a, T b) { return a > b ? a : b; }
}

template<typename T, typename F>
class alignas(alignof(T), alignof(F)) FixedResult
{
    static const size_t DATA_SIZE = Aux::StaticMax<size_t>(sizeof(T), sizeof(F));
    
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

    enum Type { Ok, Err };
    using OkData  = T;
    using ErrData = F;
    using Self    = FixedResult<T, F>;
    
    static Self Ok(const T &v) { return Self(OK_t(), v); }
    static Self Ok(T &&v) { return Self(OK_t(), std::move(v)); }
    
    static Self Err(const F &v) { return Self(ERR_t(), v); }
    static Self Err(F &&v) { return Self(ERR_t(), std::move(v)); }
    
    FixedResult(const Self &copyFrom)
    {
        isOk_ = copyFrom.isOk_;
        if(isOk_)
            new(data_) T(*static_cast<const T*>(copyFrom.data_));
        else
            new(data_) F(*static_cast<const F*>(copyFrom.data_));
    }
};

AGZ_NS_END(AGZ)
