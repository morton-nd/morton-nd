#pragma once

#include <functional>
#include <tuple>
#include <iostream>

// TODO: move to variadic_placeholder.h
template <int>
struct variadic_placeholder {};

namespace std {
	template <int N>
	struct is_placeholder<variadic_placeholder<N>>
		: integral_constant<int, N + 1>
	{
	};
}

static constexpr size_t Pow(size_t base, size_t exp) {
	return exp == 0 ? 1 : base * Pow(base, exp - 1);
}

template <typename T, size_t ...i>
static void PrintTuple(T&& tuple, std::index_sequence<i...>) {
	using expander = int[];
	(void)expander {
		0, (void(std::cout << std::get<i>(tuple) << ", "), 0)...
	};

	std::cout << std::endl;
}

template <typename F, typename Arg>
static auto Reduce(F reduce_func, Arg arg) {
	return arg;
}

template <typename F, typename Arg, typename ...Args>
static auto Reduce(F reduce_func, Arg arg, Args... args) {
	return reduce_func(arg, Reduce(reduce_func, args...));
}

template <typename Func, typename ...Args, size_t ...i>
static auto ApplyInternal(Func func, const std::tuple<Args...>& tuple, std::index_sequence<i...>) {
	return func(std::get<i>(tuple)...);
}

template <typename Func, typename ...Args>
static auto Apply(Func func, const std::tuple<Args...>& tuple) {
	return ApplyInternal(func, tuple, std::index_sequence_for<Args...>{});
}

template<typename F, typename R, typename ...ToApply, size_t MHead, size_t ...N>
static auto ApplyIndexSeqs(F function, R reduce_func, std::tuple<ToApply...> out, std::index_sequence<MHead>, std::index_sequence<N...>) {
	return Reduce(reduce_func, Apply(function, std::tuple_cat(out, std::make_tuple(N)))...);
}

template<typename F, typename R, typename ...ToApply, size_t MHead, size_t ...MTail, size_t ...N>
static auto ApplyIndexSeqs(F function, R reduce_func, std::tuple<ToApply...> out, std::index_sequence<MHead, MTail...>, std::index_sequence<N...>) {
	return Reduce(reduce_func, ApplyIndexSeqs(function, reduce_func, std::tuple_cat(out, std::make_tuple(N)), std::index_sequence<MTail...>{}, std::make_index_sequence<sizeof...(N)>{})...);
}

template<size_t N, size_t M, typename F, typename R>
static auto ApplyIndexSeqs(F function, R reduce_func) {
	return ApplyIndexSeqs(function, reduce_func, std::make_tuple<>(), std::make_index_sequence<M>{}, std::make_index_sequence<N>{});
}