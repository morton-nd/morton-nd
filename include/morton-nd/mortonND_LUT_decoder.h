//
//  mortonND_LUT_decoder.h
//  morton-nd
//
//  Copyright (c) 2015 Kevin Hartman.
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
    //static constexpr auto LutValueWidth = (LutBits + Dimensions - 1) / Dimensions;
    static constexpr auto LutValueWidth = FieldBits;

public:
    /**
     * The type used for encoding inputs as well as the result.
     */
    typedef T type;

    /**
     * The number of chunks into which each input field is partitioned. This is also
     * the number of LUT lookups performed for each field.
     *
     * For debugging / perf tuning.
     */
    static constexpr std::size_t ChunkCount = 1 + ((FieldBits * Dimensions - 1) / LutBits);

    /**
     * A mask which can be used to clear the upper bits of encoder inputs prior to
     * a call to 'Encode', if they're expected to be dirty.
     */
    constexpr T InputMask() const {
        static_assert(std::is_integral<T>::value, "Input masks are only provided for integral types.");
        return ~T(0) >> (std::numeric_limits<T>::digits - (FieldBits * Dimensions));
    }

    /**
     * The type selected internally for the LUT's value.
     * Will always accommodate Dimensions * LutBits bits.
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

    template <size_t ...I>
    constexpr auto DecodeInternal(T field, std::index_sequence<I...>) const {
        return DecodeInternal(field, I ...);
    }

//    template<typename...Args>
//    constexpr T EncodeInternal(T field1, Args... fields) const
//    {
//        return (EncodeInternal(fields...) << 1U) | LookupField(field1, std::make_index_sequence<ChunkCount>{});
//    }
//
//    constexpr T EncodeInternal(T field) const
//    {
//        return LookupField(field, std::make_index_sequence<ChunkCount>{});
//    }

//    template <size_t ...I>
//    constexpr std::array<T, Dimensions> LookupField(T field, std::index_sequence<I...>) const {
//        return LookupField(field, I...);
//    }

    template <typename ...Args>
    constexpr auto DecodeInternal(T field, std::size_t, Args... args) const
    {
//        auto chunkIndex = ChunkCount - sizeof...(args);
        auto chunkIndex = sizeof...(args);

        auto chunkStartBit = chunkIndex * LutBits;

        auto result = DecodeInternal(field, args...);
        auto chunkLookupResult = LookupTable[std::size_t((field >> chunkStartBit) & ChunkMask)];

        for (std::size_t i = 0; i < Dimensions; i++) {
            auto fieldStartIdx = chunkStartBit + i;
            auto destIdx = fieldStartIdx % Dimensions;

            // How many times have we seen this component given its stride, the number of chunks we've seen so far, and the width of each chunk?
            // (ChunkIndex * LutBits) / Dimensions

            // Example:
            // Dimensions = 3
            // LutBits = 2
            // ChunkIdx = 2
            // Code = z yx [zy] xz yx
            // LUT = [
            //   []
            // ]
            // Results:
            //   chunkStartBit = 2 * 2 = 4
            //   Dimension 0:
            //     fieldStartIdx = 4 + 0 = 4
            //     destIdx = 4 % 3 = 1
            //     insert = 4 / 2 = 2 (should this be 1?)
            //     "" = (2 * 2) / 3 = 4/3 = 1
            //   Dimension 1:
            //     fieldStartIdx = 4 + 1 = 5
            //     destIdx = 5 % 3 = 2
            //     insert = 5 / 2 = 2
            //     "" = 4 / 3
            //   Dimension 2:
            //     fieldStartIdx = 4 + 2 = 6
            //     destIdx = 6 % 3 = 0
            //     insert = 6 / 2 = 3
            auto insertOffset = fieldStartIdx / Dimensions;

            // When writing to dest, write and shift over unless you're least significant, then, just write.
            // Need to determine which components we need to write a table lookup to.
            result[destIdx] = (chunkLookupResult[i] << insertOffset) | result[destIdx];
        }

        return result;
    }

    constexpr auto DecodeInternal(T field, std::size_t) const
    {
        // This is the 0th chunk, so it lines up with the decode result array.
        return LookupTable[field & ChunkMask];
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
