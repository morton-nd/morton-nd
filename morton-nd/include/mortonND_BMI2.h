//
// Created by Kevin Hartman on 8/6/18.
//

#ifndef MORTON_ND_MORTONND_BMI2_H
#define MORTON_ND_MORTONND_BMI2_H

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
    static const std::size_t FieldBits = std::numeric_limits<T>::digits / Fields;

    template<typename...Args, typename std::enable_if<sizeof...(Args) == Fields - 1, int>::type = 0>
    static inline T Encode(T field1, Args... fields)
    {
        return EncodeInternal(field1, fields...);
    }

    static inline auto Decode(T encoding)
    {
        return DecodeInternal(encoding, std::make_index_sequence<Fields>{});
    }

private:
    MortonNDBmi() = default;

    static constexpr T BuildSelector(size_t bitsRemaining) {
        return bitsRemaining == 1 ? 1 : (BuildSelector(bitsRemaining - 1) << Fields) | 1;
    }

    static const auto Selector = BuildSelector(FieldBits);

    template<typename...Args>
    static inline T EncodeInternal(T field1, Args... fields)
    {
        return EncodeInternal(fields...) | Deposit<Fields - sizeof...(fields) - 1>(field1);
    }

    static inline T EncodeInternal(T field)
    {
        return Deposit<Fields - 1>(field);
    }

    template<size_t... i>
    static inline auto DecodeInternal(T encoding, std::index_sequence<i...>)
    {
        return std::make_tuple(Extract<i>(encoding)...);
    }

    template<size_t FieldIndex>
    static inline uint32_t Deposit(uint32_t field) {
        return _pdep_u32(field, Selector << FieldIndex);
    }

    template<size_t FieldIndex>
    static inline uint64_t Deposit(uint64_t field) {
        return _pdep_u64(field, Selector << FieldIndex);
    }

    template<size_t FieldIndex>
    static inline uint32_t Extract(uint32_t encoding) {
        return _pext_u32(encoding, Selector << FieldIndex);
    }

    template<size_t FieldIndex>
    static inline uint64_t Extract(uint64_t encoding) {
        return _pext_u64(encoding, Selector << FieldIndex);
    }
};
}

#endif
#endif //MORTON_ND_MORTONND_BMI2_H
