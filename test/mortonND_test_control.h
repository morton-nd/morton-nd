#pragma once

#include <limits>
#include <valarray>
#include <functional>
#include <type_traits>

template<std::size_t FieldCount>
uint64_t split_by_n(uint64_t input, size_t bitsRemaining) {
    return (bitsRemaining == 0) ? input : (split_by_n<FieldCount>(input >> 1, bitsRemaining - 1) << FieldCount) | (input & (uint64_t)1);
}

template<std::size_t FieldCount>
uint64_t join_by_n(uint64_t input, size_t bitsRemaining) {
    return (bitsRemaining == 0) ? 0 : (join_by_n<FieldCount>(input >> FieldCount, bitsRemaining - 1) << 1) | (input & (uint64_t)1);
}

template<std::size_t FieldCount>
uint64_t control_encode_impl(std::size_t bitCount, const std::valarray<uint64_t>& fields) {
    if (fields.size() == 1) {
        return split_by_n<FieldCount>(fields[std::slice(0, 1, 1)][0], bitCount);
    }

    return (control_encode_impl<FieldCount>(bitCount, fields[std::slice(1, fields.size() - 1, 1)]) << 1)
        | split_by_n<FieldCount>(fields[std::slice(0, 1, 1)][0], bitCount);
}

/**
 * Runtime Morton encoding using 64-bit fields. Only the first floor(64 / |fields|) bits will be encoded from each field.
 *
 * @param fields The fields to encode.
 * @return The Morton encoding.
 */
template<typename...T>
uint64_t control_encode(T... fields) {
    return control_encode_impl<sizeof...(fields)>(std::numeric_limits<uint64_t>::digits / sizeof...(fields), { fields... });
}

template<std::size_t FieldCount>
void control_decode_impl(uint64_t encoding, std::size_t bitCount, const std::valarray<std::reference_wrapper<uint64_t>>& fields) {
    if (fields.size() == 1) {
        fields[std::slice(0, 1, 1)][0].get() = join_by_n<FieldCount>(encoding, bitCount);
        return;
    }

    fields[std::slice(0, 1, 1)][0].get() = join_by_n<FieldCount>(encoding, bitCount);
    control_decode_impl<FieldCount>(encoding >> 1, bitCount, fields[std::slice(1, fields.size() - 1, 1)]);
}

template<typename...T>
void control_decode(uint64_t encoding, T&... fields) {
    const std::valarray<std::reference_wrapper<uint64_t>> list = { fields... };
    return control_decode_impl<sizeof...(fields)>(encoding, std::numeric_limits<uint64_t>::digits / sizeof...(fields), list);
}