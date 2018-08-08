//
// Created by Kevin Hartman on 8/6/18.
//

#ifndef MORTON_ND_MORTONND_BMI2_ENCODER_H
#define MORTON_ND_MORTONND_BMI2_ENCODER_H

#include <array>
#include <cmath>
#include <limits>
#include <type_traits>
#include <immintrin.h>

namespace mortonnd{

template<std::size_t Fields, typename T,
    typename = std::enable_if_t<std::is_same<T, uint32_t>::value | std::is_same<T, uint64_t>::value>>
class MortonNDBmiEncoder
{
public:
    template<typename...Args, typename std::enable_if<sizeof...(Args) == Fields - 1, int>::type = 0>
    inline static T Encode(T field1, Args... fields)
    {
        return EncodeInternal(field1, fields...);
    }

private:
    static const std::size_t FieldBits = std::numeric_limits<T>::digits / Fields;

    template<typename...Args>
    static inline T EncodeInternal(T field1, Args... fields)
    {
        return EncodeInternal(fields...) | Deposit(field1, Fields - sizeof...(fields) - 1);
    }

    static inline T EncodeInternal(T field)
    {
        return Deposit(field, Fields - 1);
    }

    static inline uint32_t Deposit(uint32_t field, size_t selectorIndex) {
        return _pdep_u32(field, Selectors[selectorIndex]);
    }

    static inline uint64_t Deposit(uint64_t field, size_t selectorIndex) {
        return _pdep_u64(field, Selectors[selectorIndex]);
    }

    static constexpr T BuildSelector(size_t bitsRemaining) {
        return bitsRemaining == 1 ? 1 : (BuildSelector(bitsRemaining - 1) << Fields) | 1;
    }

    template<size_t... i>
    static constexpr auto BuildSelectorMasks(std::index_sequence<i...>) {
        return std::array<T, sizeof...(i)>{{(BuildSelector(FieldBits) << i)...}};
    }

    static constexpr std::array<T, Fields> Selectors = BuildSelectorMasks(std::make_index_sequence<Fields>{});
};
}

#endif //MORTON_ND_MORTONND_BMI2_ENCODER_H
