//
//  mortonND_LUT_decoder.h
//  morton-nd
//
//  Copyright (c) 2021 Kevin Hartman.
//

#ifndef mortonND_decoder_h
#define mortonND_decoder_h

#include "mortonND_LUT_encoder.h"

#include <cmath>
#include <array>
#include <tuple>
#include <type_traits>
#include <limits>

namespace mortonnd {

// TODO: make sure we won't ever shift input >= digits of size_t
template<std::size_t BitsRemaining>
constexpr auto JoinByN(std::size_t input, std::size_t fields) {
    return (JoinByN<BitsRemaining - 1>(input >> fields, fields) << 1U) | (input & 1U);
}

/**
 * Special case to avoid shifting >= width of 'input'.
 */
template<>
constexpr auto JoinByN<1>(std::size_t input, std::size_t) {
    return input & 1U;
}

template<std::size_t Dimensions, std::size_t FieldBits, std::size_t LutBits, typename T = FastInt<Dimensions * FieldBits>>
class MortonNDLutDecoder
{
    constexpr static MortonNDLutValidator<Dimensions, FieldBits, LutBits, T> validate {};

    // TODO:
    //  - THis should be used instead, since elements don't need to hold full size of fields,
    //    but for now, we use field size for simplicity.
    //  - Validate math.
    static constexpr auto LutValueWidth = (LutBits + Dimensions - 1) / Dimensions;
    //static constexpr auto LutValueWidth = FieldBits;

public:
    /**
     * The type used for encoding inputs as well as the result.
     */
    typedef T type;

    /**
     * The number of chunks into which the input Morton code is partitioned,
     * i.e. the number of LUT lookups performed when decoding.
     *
     * For debugging / perf tuning.
     */
    static constexpr std::size_t ChunkCount = 1 + ((FieldBits * Dimensions - 1) / LutBits);

    /**
     * A mask which can be used to clear the upper bits of the input Morton code prior to
     * a call to 'Decode', if they're expected to be dirty.
     */
    constexpr T InputMask() const {
        static_assert(std::is_integral<T>::value, "Input masks are only provided for integral types.");
        return ~T(0) >> (std::numeric_limits<T>::digits - (FieldBits * Dimensions));
    }

    /**
     * The type selected internally LUT entry array values.
     * I.e. Each LUT entry is of type std::array<LutValue, Dimensions>.
     *
     * For debugging / perf tuning.
     */
    using LutValue = MinInt<LutValueWidth>;

    /**
     * Constexpr constructor.
     *
     * The resulting class literal instance holds the generated LUT (configurable via class
     * template parameters) and provides a 'Decode' function which is optimized to perform
     * decodings using that LUT.
     *
     * Note that the 'Decode' function is also constexpr, and can be used to decode
     * at compile-time.
     */
    constexpr MortonNDLutDecoder() = default;

    constexpr auto Decode(T input) const
    {
        return DecodeInternal(input, std::make_index_sequence<ChunkCount>{});
    }

private:
    template<std::size_t ChunkStartBit, typename ResultTuple, std::size_t ...I>
    constexpr auto MapComponents(ResultTuple& result, const std::array<LutValue, Dimensions>& chunkLookupResult, std::index_sequence<I...>) const {
        MapComponents<ChunkStartBit>(result, chunkLookupResult, I...);
    }

    template<std::size_t ChunkStartBit, typename ResultTuple, typename ...Args>
    constexpr void MapComponents(ResultTuple& result, const std::array<LutValue, Dimensions>& chunkLookupResult, std::size_t, Args... args) const {
        InjectBits<ChunkStartBit, sizeof...(Args)>(result, chunkLookupResult);
        MapComponents<ChunkStartBit>(result, chunkLookupResult, args...);
    }

    template<std::size_t ChunkStartBit, typename ResultTuple, typename ...Args>
    constexpr void MapComponents(ResultTuple& result, const std::array<LutValue, Dimensions>& chunkLookupResult, std::size_t) const {
        InjectBits<ChunkStartBit, 0>(result, chunkLookupResult);
    }

    template<std::size_t ChunkStartBit, std::size_t SourceIndex, typename ResultTuple>
    constexpr void InjectBits(ResultTuple& result, const std::array<LutValue, Dimensions>& chunkLookupResult) const {
        constexpr auto FieldStartIndex = ChunkStartBit + SourceIndex;
        constexpr auto DestIndex = FieldStartIndex % Dimensions;
        constexpr auto InsertOffset = FieldStartIndex / Dimensions;
        std::get<DestIndex>(result) = (((T)chunkLookupResult[SourceIndex]) << InsertOffset) |((T)std::get<DestIndex>(result));
    }

    /**
     * Algorithm steps:
     *   - Split 'field' into chunks of width 'LutBits'.
     *   - For each chunk, look up the chunk in 'LookupTable', which returns an array of
     *      its de-interleaved components. E.g.: LookupTable[yxzyxzyx] => [xxx, yyy, zz]
     *        - For each component,
     *           - Determine the destination field to which the component belongs. This is calculated statically
     *             as (ChunkStartBit + ComponentIndex) % Dimensions.
     *           - Inject the component's bits at the current write offset for the destination field.
     *             This is calculated statically as (ChunkStartBit + ComponentIndex) / Dimensions.
     *
     * Example:
     *   Dimensions: 3
     *   Input:      zyxzyxzyxzyx
     *   LutBits:    5
     *
     *   Chunks: [ yxzyx, xzyxz, zy ]
     *
     *   - Chunks[0]:
     *     LookupResult = Lookup[yxzyx] => [xx, yy, z]:
     *     - LookupResult[0] => xx
     *       DestIndex =   (0 + 0) % 3 => 0
     *       InsertIndex = (0 + 0) / 3 => 0
     *       In English: Write xx at bit 0 of field 0.
     *
     *     - LookupResult[1] => yy
     *       DestIndex =   (0 + 1) % 3 => 1
     *       InsertIndex = (0 + 1) / 3 => 0
     *       In English: Write yy at bit 0 of field 1.
     *
     *     - LookupResult[2] => z
     *       DestIndex =   (0 + 2) % 3 => 2
     *       InsertIndex = (0 + 2) / 3 => 0
     *       In English: Write z at bit 0 of field 2.
     *
     *   Result so far: (xx, yy, z)
     *                   ^^  ^^  ^
     *   - Chunks[1]:
     *     LookupResult = Lookup[xzyxz] => [zz, xx, y]:
     *     - LookupResult[0] => zz
     *       DestIndex =   (5 + 0) % 3 => 2
     *       InsertIndex = (5 + 0) / 3 => 1
     *       In English: Write zz at bit 1 of field 2.
     *
     *     - LookupResult[1] => xx
     *       DestIndex =   (5 + 1) % 3 => 0
     *       InsertIndex = (5 + 1) / 3 => 2
     *       In English: Write xx at bit 2 of field 0.
     *
     *     - LookupResult[2] => y
     *       DestIndex =   (5 + 2) % 3 => 1
     *       InsertIndex = (5 + 2) / 3 => 2
     *       In English: Write y at bit 2 of field 1.
     *
     *   Result so far: (xxxx, yyy, zzz)
     *                   ^^    ^    ^^
     *   - Chunks[2]:
     *     LookupResult = Lookup[000zy] => [y, z, 0]:
     *     - LookupResult[0] => y
     *       DestIndex =   (10 + 0) % 3 => 1
     *       InsertIndex = (10 + 0) / 3 => 3
     *       In English: Write y at bit 3 of field 1.
     *
     *     - LookupResult[1] => z
     *       DestIndex =   (10 + 1) % 3 => 2
     *       InsertIndex = (10 + 1) / 3 => 3
     *       In English: Write z at bit 3 of field 2.
     *
     *     - LookupResult[2] => 0
     *       DestIndex =   (10 + 2) % 3 => 0
     *       InsertIndex = (10 + 2) / 3 => 4
     *       In English: Write 0 at bit 4 of field 0.
     *
     *   Final result: (0xxxx, yyyy, zzzz)
     *                  ^      ^     ^
     * @tparam Args
     * @param field
     * @param args
     * @return
     */
    template <typename ...Args>
    constexpr auto DecodeInternal(T field, std::size_t, Args... args) const
    {
        auto constexpr ChunkIndex = sizeof...(args);
        auto constexpr ChunkStartBit = ChunkIndex * LutBits;

        auto result = DecodeInternal(field, args...);
        auto chunkLookupResult = LookupTable[std::size_t((field >> ChunkStartBit) & ChunkMask)];

        // 'MapComponents' equivalent logic:
        //
        // for (std::size_t i = 0; i < Dimensions; i++) {
        //     auto fieldStartIdx = ChunkStartBit + i;
        //     auto destIdx = fieldStartIdx % Dimensions;
        //     auto insertOffset = fieldStartIdx / Dimensions;
        //     result[destIdx] = (chunkLookupResult[i] << insertOffset) | result[destIdx];
        // }
        MapComponents<ChunkStartBit>(result, chunkLookupResult, std::make_index_sequence<Dimensions>{});

        return result;
    }

    template <size_t ...I>
    constexpr auto DecodeInternal(T field, std::index_sequence<I...>) const {
        return DecodeInternal(field, I ...);
    }

    template <typename Array, std::size_t ...I>
    constexpr auto CreateTuple(Array arr, std::index_sequence<I...>) const {
        return std::make_tuple((T)arr[I]...);
    }

    constexpr auto DecodeInternal(T field, std::size_t) const
    {
        // This is the 0th chunk, so it lines up with the decode result array.
        return CreateTuple(LookupTable[field & ChunkMask], std::make_index_sequence<Dimensions>{});
    }

    // NOTE: this is implemented at namespace level due to CWG727.
    // http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#727
    static constexpr LutValue JoinByN(std::size_t input) {
        return mortonnd::JoinByN<FieldBits>(input, Dimensions);
    }

    template<size_t... i>
    static constexpr auto BuildLutEntry(std::size_t input, std::index_sequence<i...>) {
        return std::array<LutValue, Dimensions>{{JoinByN(input >> i) ...}};
    }

    template<size_t... i>
    static constexpr auto BuildLut(std::index_sequence<i...>) noexcept {
        return std::array<std::array<LutValue, Dimensions>, sizeof...(i)>{{BuildLutEntry(i, std::make_index_sequence<Dimensions>{})...}};
    }

    static constexpr std::size_t pow(std::size_t base, std::size_t exp) {
        return exp == 0 ? 1 : base * pow(base, exp - 1);
    }

    static constexpr std::size_t ComputeLutSize() {
        return pow(2, LutBits);
    }
public:
    static constexpr std::size_t LutSize = ComputeLutSize();
    static constexpr std::size_t ChunkMask = ~std::size_t(0) >> (std::numeric_limits<std::size_t>::digits - LutBits);
    const std::array<std::array<LutValue, Dimensions>, LutSize> LookupTable = BuildLut(std::make_index_sequence<LutSize>{});
};
}
#endif
