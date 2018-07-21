//
//  mortonND_encoder.h
//  morton-nd
//
//  Created by Kevin Hartman on 2/23/15.
//  Copyright (c) 2015 Kevin Hartman. All rights reserved.
//

#ifndef mortonND_encoder_h
#define mortonND_encoder_h

#include <cmath>
#include <array>
#include <tuple>
#include <type_traits>
#include <limits>

namespace mortonnd{

template<size_t Size, typename Default = void()>
using built_in_t =
    typename std::conditional<(Size <= std::numeric_limits<uint8_t>::digits), uint8_t,
        typename std::conditional<(Size <= std::numeric_limits<uint16_t>::digits), uint16_t,
            typename std::conditional<(Size <= std::numeric_limits<uint32_t>::digits), uint32_t,
                typename std::conditional<(Size <= std::numeric_limits<uint64_t>::digits), uint64_t, Default>
                    ::type>::type>::type>::type;

/**
 * @param Fields the number of fields (components) to encode
 * @param FieldBits the number of bits in each input field, starting with the LSb. Higher bits are not cleared.
 * @param LutBits the number of bits for the LUT. Each field will be looked-up 'LutBits' bits at a time.
 * @param T the type of the components to encode/decode, as well as the type of the result
 */
template<std::size_t Fields, std::size_t FieldBits, std::size_t LutBits, typename T = built_in_t<Fields * FieldBits>>
class MortonNDEncoder
{
    // LUT entry size is always Fields * LutBits. If no suitable built-in type can hold the entry, the user-specified field
    // type will be used (if provided).
    using lut_entry_t = built_in_t<Fields * LutBits, T>;

public:
    static const std::size_t ChunkCount = 1 + ((FieldBits - 1) / LutBits);
    static const T InputMask = ((T)1 << FieldBits) - 1;

    constexpr MortonNDEncoder(): LookupTable(BuildLut(std::make_index_sequence<LutSize()>{})) {}

    template<typename...Args, typename std::enable_if<sizeof...(Args) == Fields - 1, int>::type = 0>
    constexpr T Encode(T field1, Args... fields) const
    {
        return EncodeInternal(field1, fields...);
    }

private:
    template<typename...Args>
    constexpr T EncodeInternal(T field1, Args... fields) const
    {
        return (EncodeInternal(fields...) << 1) | LookupField(field1, std::make_index_sequence<ChunkCount>{});
    }

    constexpr T EncodeInternal(T field) const
    {
        return LookupField(field, std::make_index_sequence<ChunkCount>{});
    }

    template <size_t ...I>
    constexpr T LookupField(T field, std::index_sequence<I...>) const {
        return LookupField(field, I...);
    }

    template <typename ...Args>
    constexpr T LookupField(T field, size_t, Args... args) const
    {
        return (LookupField(field >> LutBits, args...) << (Fields * LutBits)) | LookupTable[field & ChunkMask];
    }

    constexpr T LookupField(T field, size_t) const
    {
        return LookupTable[field & ChunkMask];
    }

    static constexpr lut_entry_t SplitByN(lut_entry_t input, size_t bitsRemaining = LutBits) {
        static_assert(Fields > 0, "Field parameter (# fields) must be > 0");

        return (bitsRemaining == 0)
            ? input
            : (SplitByN(input >> 1, bitsRemaining - 1) << Fields) | (input & (lut_entry_t)1);
    }

    template<size_t... i>
    static constexpr auto BuildLut(std::index_sequence<i...>) {
        return std::array<lut_entry_t, sizeof...(i)>{{SplitByN(i)...}};
    }

    static constexpr size_t pow(size_t base, size_t exp) {
        return exp == 0 ? 1 : base * pow(base, exp - 1);
    }

    static constexpr size_t LutSize() {
        return pow(2, LutBits);
    }

    static const T ChunkMask = ((T)1 << LutBits) - 1;
    const std::array<lut_entry_t, LutSize()> LookupTable;
};
} // namespace mortonnd
#endif
