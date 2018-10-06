#pragma once

#include <functional>
#include <tuple>
#include <iostream>

static constexpr size_t Pow(size_t base, size_t exp) {
    return exp == 0 ? 1 : base * Pow(base, exp - 1);
}

template<size_t idx, typename T>
struct tuple_elem_t {
    typedef T type;
};

template<typename ...T>
struct type_sequence { };

template<typename T, size_t...i>
static auto make_type_sequence(std::index_sequence<i...>) {
    return type_sequence<typename tuple_elem_t<i, T>::type...>{};
}

template<size_t size, typename T>
static auto make_type_sequence() {
    return make_type_sequence<T>(std::make_index_sequence<size>{});
}

template<typename T, size_t ...i>
static void PrintTuple(T&& tuple, std::index_sequence<i...>) {
    using expander = int[];
    (void)expander{
        0, (void(std::cout << std::get<i>(tuple) << ", "), 0)...
    };

    std::cout << std::endl;
}

template<typename F, typename Arg>
static auto Reduce(F, Arg arg) {
    return arg;
}

template<typename F, typename Arg, typename ...Args>
static auto Reduce(F reduce_func, Arg arg, Args... args) {
    return reduce_func(arg, Reduce(reduce_func, args...));
}

template<typename Func, typename ...Args, size_t ...i>
static auto ApplyInternal(Func func, const std::tuple<Args...>& tuple, std::index_sequence<i...>) {
    return func(std::get<i>(tuple)...);
}

template<typename Func, typename ...Args>
static auto Apply(Func func, const std::tuple<Args...>& tuple) {
    return ApplyInternal(func, tuple, std::index_sequence_for<Args...>{});
}

template<typename Func, typename ...Tuples, size_t ...i>
static auto ApplyEach(Func func, const std::tuple<Tuples...>& tups, std::index_sequence<i...>) {
    return std::make_tuple(Apply(func, std::get<i>(tups))...);
};

template<typename Func, typename ...Tuple>
static auto ApplyEach(Func func, const std::tuple<Tuple...>& tups) {
    return ApplyEach(func, tups, std::index_sequence_for<Tuple...>{});
};

template<typename T, typename F, typename R, typename ...ToApply, T ...N>
static auto ApplyIndexSeqs(F function, R reduce_func, std::integral_constant<size_t, 0>, std::tuple<ToApply...> out, std::integer_sequence<T, N...>) {
    return Reduce(reduce_func, Apply(function, std::tuple_cat(out, std::make_tuple(N)))...);
}

template<typename T, typename F, typename R, size_t M, typename ...ToApply, T ...N>
static auto ApplyIndexSeqs(F function, R reduce_func, std::integral_constant<size_t, M>, std::tuple<ToApply...> out, std::integer_sequence<T, N...>) {
    return Reduce(reduce_func, ApplyIndexSeqs(
        function, reduce_func, std::integral_constant<size_t, M - 1>{}, std::tuple_cat(out, std::make_tuple(N)), std::make_integer_sequence<T, sizeof...(N)>{}
    )...);
}

template<typename T, T N, size_t M, typename F, typename R>
static auto ApplyIndexSeqs(F function, R reduce_func) {
    return ApplyIndexSeqs(function, reduce_func, std::integral_constant<size_t, M - 1>{}, std::make_tuple<>(), std::make_integer_sequence<T, N>{});
}

template<typename ...Fields, size_t ...i>
static auto Tail(std::tuple<Fields...> tup, std::index_sequence<0, i...>) {
    return std::make_tuple(std::get<i>(tup)...);
}

template<typename ...Fields>
static auto Tail(std::tuple<Fields...> tup) {
    return Tail(tup, std::make_index_sequence<sizeof...(Fields)>{});
}

template<typename Field1, typename Field2>
static auto Zip(std::tuple<Field1> tup1, std::tuple<Field2> tup2) {
    return std::make_tuple(std::tuple_cat(tup1, tup2));
}

template<typename Field1, typename Field2, typename ...Fields1, typename ...Fields2>
static auto Zip(std::tuple<Field1, Fields1...> tup1, std::tuple<Field2, Fields2...> tup2) {
    return std::tuple_cat(
        std::make_tuple(std::make_tuple(std::get<0>(tup1), std::get<0>(tup2))),
        Zip(Tail(tup1), Tail(tup2))
    );
}

template<typename ...T, size_t ...i>
static auto RefTuple(std::tuple<T...>& tup, std::index_sequence<i...>) {
    return std::make_tuple(std::ref(std::get<i>(tup))...);
}

template<typename ...T>
static auto RefTuple(std::tuple<T...>& tup) {
    return RefTuple(tup, std::index_sequence_for<T...>{});
}