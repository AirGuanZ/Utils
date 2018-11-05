#pragma once

#include "../Misc/Common.h"
#include "../Misc/TypeOpr.h"

namespace AGZ::Math {

using DimType = uint32_t;

/**
 * @brief 编译时任意维向量
 */
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
    auto EachElemLessThanAux(const Vec<DIM, T> &rhs, std::integer_sequence<DimType, Indices...>) const
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

	/**
	 * 默认初始化为零向量
	 */
    Vec() : Vec(T(0)) { }

	/**
	 * 不对元素进行任何初始化
	 */
    explicit constexpr Vec(Uninitialized_t) { }

	/**
	 * 将所有元素初始化为同一个值
	 */
    explicit Vec(const Element &value) noexcept
        : Vec(UNINITIALIZED)
    {
        static_assert(noexcept(Element(value)));
        for(auto &e : data)
            new(&e) Element(value);
    }

	/**
	 * 分别指定每个元素的值
	 */
    template<typename...Args, int = 0,
             typename = TypeOpr::TrueToVoid_t<
                (Dim > 1) &&
                (TypeOpr::TypeListLength_v<Args...> == Dim) &&
                !TypeOpr::Any_v<CanConvertToUninitializedFlag, Args...>>>
    constexpr Vec(Args&&...args)
        : data{ Element(std::forward<Args>(args))... }
    {

    }

	/**
	 * 按下标取得特定元素值
	 */
    Element       &operator[](size_t idx)       { AGZ_ASSERT(idx < Dim); return data[idx]; }
    
	//! @copydoc Vec<DIM, T>::operator[](size_t idx)
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

	/**
	 * 求所有元素的乘积
	 */
    auto Product() const
    {
        return ProductAux(std::make_integer_sequence<DimType, Dim>());
    }

	/**
	 * 求所有元素的和
	 */
    auto Sum() const
    {
        return SumAux(std::make_integer_sequence<DimType, Dim>());
    }

	/**
	 * 是否每个分量都小于另一向量的对应分量
	 */
    auto EachElemLessThan(const Self &rhs) const
    {
        return EachElemLessThanAux(rhs, std::make_integer_sequence<DimType, Dim>());
    }
};

} // namespace AGZ::Math
