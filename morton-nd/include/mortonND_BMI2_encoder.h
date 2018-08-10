//
// Created by Kevin Hartman on 8/6/18.
//

#ifndef MORTON_ND_MORTONND_BMI2_ENCODER_H
#define MORTON_ND_MORTONND_BMI2_ENCODER_H

#if defined(__BMI2__)
#include <array>
#include <cmath>
#include <limits>
#include <type_traits>
#include <immintrin.h>

namespace mortonnd{

template<std::size_t Fields, typename T,
    typename = std::enable_if_t<std::is_same<T, uint32_t>::value | std::is_same<T, uint64_t>::value>>
class MortonNDBmi
{
public:
    constexpr MortonNDBmi() = default;

    template<typename...Args, typename std::enable_if<sizeof...(Args) == Fields - 1, int>::type = 0>
    inline T Encode(T field1, Args... fields) const
    {
        return EncodeInternal(field1, fields...);
    }

    inline auto Decode(T encoding) const
    {
        return DecodeInternal(encoding, std::make_index_sequence<Fields>{});
    }

private:
    static const std::size_t FieldBits = std::numeric_limits<T>::digits / Fields;

    template<typename...Args>
    inline T EncodeInternal(T field1, Args... fields) const
    {
        return EncodeInternal(fields...) | Deposit<Fields - sizeof...(fields) - 1>(field1);
    }

    inline T EncodeInternal(T field) const
    {
        return Deposit<Fields - 1>(field);
    }

    template<size_t... i>
    inline auto DecodeInternal(T encoding, std::index_sequence<i...>) const
    {
        return std::make_tuple(Extract<i>(encoding)...);
    }

    template<size_t Index>
    inline uint32_t Deposit(uint32_t field) const {
        return _pdep_u32(field, Selectors[Index]);
    }

    template<size_t Index>
    inline uint64_t Deposit(uint64_t field) const {
        return _pdep_u64(field, Selectors[Index]);
    }

    template<size_t Index>
    inline uint32_t Extract(uint32_t encoding) const {
        return _pext_u32(encoding, Selectors[Index]);
    }

    template<size_t Index>
    inline uint64_t Extract(uint64_t encoding) const {
        return _pext_u64(encoding, Selectors[Index]);
    }

    static constexpr T BuildSelector(size_t bitsRemaining) {
        return bitsRemaining == 1 ? 1 : (BuildSelector(bitsRemaining - 1) << Fields) | 1;
    }

    template<size_t... i>
    static constexpr auto BuildSelectorMasks(std::index_sequence<i...>) {
        return std::array<T, sizeof...(i)>{{(BuildSelector(FieldBits) << i)...}};
    }

    const std::array<T, Fields> Selectors = BuildSelectorMasks(std::make_index_sequence<Fields>{});
};
}

#endif
#endif //MORTON_ND_MORTONND_BMI2_ENCODER_H
