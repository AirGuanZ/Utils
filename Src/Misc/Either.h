#pragma once

#include <type_traits>

#include "Common.h"
#include "TypeOpr.h"

AGZ_NS_BEG(AGZ)

template<typename L, typename R, std::enable_if_t<!std::is_same_v<L, R>, int> = 0>
class Either
{
	TypeOpr::Variant<L, R> var_;

public:

	using Self = Either<L, R>;

	template<typename U, std::enable_if_t<std::is_same_v<remove_rcv_t<U>, L>, int> = 0>
	Either(U &&u) : var_(std::forward<U>(u)) { }

	template<typename U, std::enable_if_t<std::is_same_v<remove_rcv_t<U>, R>, int> = 0>
	Either(U &&u): var_(std::forward<U>(u)) { }

	explicit Either(const Self &copyFrom) : var_(copyFrom.var_) { }

	explicit Either(Self &&moveFrom) noexcept : var_(std::move(moveFrom.var_)) { }

	~Either() = default;

	Self &operator=(const Self &copyFrom) { var_ = copyFrom.var_; return *this; }

	Self &operator=(Self &&moveFrom) noexcept { var_ = std::move(moveFrom.var_); return *this; }

	bool IsLeft() const { return std::get_if<0>(&var_) != nullptr; }

	bool IsRight() const { return std::get_if<1>(&var_) != nullptr; }

	L &GetLeft() { return std::get<0>(var_); }

	const L &GetLeft() const { return std::get<0>(var_); }

	R &GetRight() { return std::get<1>(var_); }

	const R &GetRight() const { return std::get<1>(var_); }
};

AGZ_NS_END(AGZ)
