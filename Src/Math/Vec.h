#pragma once

#include "../Misc/Common.h"
#include "../Misc/TypeOpr.h"

AGZ_NS_BEG(AGZ::Math)

using DimType = uint32_t;

template<DimType DIM, typename T>
class Vec
{
    template<DimType...Indices>
    auto ProductAux(std::integer_sequence<DimType, Indices...>) const
    {
        return (... * data[Indices]);
    }

    template<DimType...Indices>
    auto SumAux(std::integer_sequence<DimType, Indices...>) const
    {
        return (... + data[Indices]);
    }

    template<DimType...Indices>
    auto EachLessThanAux(const Vec<DIM, T> &rhs, std::integer_sequence<DimType, Indices...>) const
    {
        return (... && (data[Indices] < rhs[Indices]));
    }

public:

    using Element = T;
    using Self    = Vec<DIM, T>;

    using DimType                = ::AGZ::Math::DimType;
    static constexpr DimType Dim = DIM;

    static_assert(Dim > 0);
    static_assert(std::is_trivially_destructible_v<T>);

    Element data[Dim];

    Vec() : Vec(T(0)) { }

    explicit constexpr Vec(Uninitialized_t) { }

    explicit Vec(const Element &value) noexcept
        : Vec(UNINITIALIZED)
    {
        static_assert(noexcept(Element(value)));
        for(auto &e : data)
            new(&e) Element(value);
    }

    template<typename...Args,
             typename = TypeOpr::TrueToVoid_t<
                (Dim > 1) &&
                (TypeOpr::TypeListLength_v<Args...> == Dim) &&
                !TypeOpr::Any_v<CanConvertToUninitializedFlag, Args...>>>
    constexpr Vec(Args&&...args)
        : data{ static_cast<Element>(std::forward<Args>(args))... }
    {

    }

    Element       &operator[](size_t idx)       { AGZ_ASSERT(idx < Dim); return data[idx]; }
    const Element &operator[](size_t idx) const { AGZ_ASSERT(idx < Dim); return data[idx]; }

    bool operator==(const Self &rhs) const
    {
        for(DimType i = 0; i < Dim; ++i)
        {
            if(data[i] != rhs.data[i])
                return false;
        }
        return true;
    }

    bool operator!=(const Self &rhs) const
    {
        return !(*this == rhs);
    }

    auto Product() const
    {
        return ProductAux(std::make_integer_sequence<DimType, Dim>());
    }

    auto Sum() const
    {
        return SumAux(std::make_integer_sequence<DimType, Dim>());
    }

    auto EachLessThan(const Self &rhs) const
    {
        return EachLessThanAux(rhs, std::make_integer_sequence<DimType, Dim>());
    }
};

AGZ_NS_END(AGZ::Math)
