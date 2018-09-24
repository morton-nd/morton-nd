#pragma once

#include <type_traits>

template<int>
struct variadic_placeholder { };

namespace std {
template<int N>
struct is_placeholder<variadic_placeholder<N>>
    : integral_constant<int, N + 1> {
};
}