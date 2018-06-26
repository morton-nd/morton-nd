//
//  mortoncode.h
//  MetaMorton
//
//  Created by Kevin Hartman on 2/23/15.
//  Copyright (c) 2015 Kevin Hartman. All rights reserved.
//

#ifndef MetaMorton_mortoncode_h
#define MetaMorton_mortoncode_h

#include <cmath>
#include <array>
#include <tuple>
#include <type_traits>
#include <limits>

template<size_t Size, typename Default = void()>
using built_in_t =
    typename std::conditional<(Size <= std::numeric_limits<uint8_t>::digits), uint8_t,
        typename std::conditional<(Size <= std::numeric_limits<uint16_t>::digits), uint16_t,
            typename std::conditional<(Size <= std::numeric_limits<uint32_t>::digits), uint32_t,
                typename std::conditional<(Size <= std::numeric_limits<uint64_t>::digits), uint64_t, Default>::type>::type>::type>::type;

/**
 * @param Fields the number of fields (components) to encode/decode
 * @param Chunks the number of chunks per field
 * @param Bits the number of bits per chunk
 * @param T the type of the components to encode/decode, as well as the type of the result
 */
template<std::size_t Fields, std::size_t Chunks, std::size_t Bits, typename T = built_in_t<Fields * Chunks * Bits>>
class MortonCode
{
    // LUT entry size is always Fields * Bits. If no suitable built-in type can hold the entry, the user-specified field
    // type will be used (if provided).
    using lut_entry_t = built_in_t<Fields * Bits, T>;

public:
    constexpr MortonCode(): LookupTable(BuildLut(std::make_index_sequence<LutSize()>{})) {}

    template<typename...Args, typename std::enable_if<sizeof...(Args) == Fields - 1, int>::type = 0>
    constexpr T Encode(T field1, Args... fields) const
    {
        return EncodeInternal(field1, fields...);
    }

private:
    template<typename...Args>
    constexpr T EncodeInternal(T field1, Args... fields) const
    {
        return (EncodeInternal(fields...) << 1) | LookupField(field1, std::make_index_sequence<Chunks>{});
    }

    constexpr T EncodeInternal(T field) const
    {
        return LookupField(field, std::make_index_sequence<Chunks>{});
    }

    template <typename Arg, typename...Args>
    constexpr auto LookupField(T field, Arg, Args... args) const
    {
        return (LookupField(field >> Bits, args...) << (Fields * Bits)) | LookupTable[field & ChunkMask];
    }

    template <typename Arg>
    constexpr auto LookupField(T field, Arg) const
    {
        return LookupTable[field & ChunkMask]; // TODO: no need to mask here assuming clean input
    }

    static constexpr lut_entry_t SplitByN(lut_entry_t input, size_t bitsRemaining = Bits) {
        static_assert(Fields > 0, "Field parameter (# fields) must be > 0");

        return (bitsRemaining == 0) ? input : (SplitByN(input >> 1, bitsRemaining - 1) << Fields) | (input & 1);
    }

    template<size_t... i>
    static constexpr auto BuildLut(std::index_sequence<i...>) {
        return std::array<lut_entry_t, sizeof...(i)>{{SplitByN(i)...}};
    }

    static constexpr size_t pow(size_t base, size_t exp) {
        return exp == 0 ? 1 : base * pow(base, exp - 1);
    }

    static constexpr size_t LutSize() {
        return pow(2, Bits);
    }

    const std::array<lut_entry_t, LutSize()> LookupTable;
    const lut_entry_t ChunkMask = ((lut_entry_t)1 << Bits) - 1;
};

#endif
