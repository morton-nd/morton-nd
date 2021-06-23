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

/**
 * The mapping function used by lookup table entry generation.
 *
 * Extract every 'fields' bits from 'input', depositing each to the
 * result, filling from LSb to MSb without gaps.
 *
 * Example ('fields' == 3):
 *   73 (dec) => 1001001 (bin) => 111 (bin) => 7 (dec)
 *
 * @tparam BitsRemaining the number of least-significant-bits left to process.
 * @param input the source integer.
 * @param fields the stride between bits to extract from the source.
 * @return the extracted component of 'input'.
 */
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

/**
 * A fast portable N-dimensional LUT-based Morton decoder.
 *
 * This class literal (constexpr class) generates a suitable LUT (based on template parameters)
 * along with an efficient (i.e. no func calls, no branches) Morton decoding algorithm.
 *
 * This implementation supports up to 128-bit Morton codes using native integer types, but
 * can be also be used with user-provided encoding types (e.g. a BigInteger class) to support
 * encodings of any size. Note that user-provided encoding types must define the standard C++
 * integral operators, and additionally must be able to construct a std::size_t.
 *
 * Configuration:
 *
 * Dimensions
 *   You must define the number of dimensions with which this decoder instance
 *   will be used via template parameter. The resulting instance's 'Decode' function will
 *   return an std::tuple of this cardinality.
 *
 * FieldBits
 *   The number of bits (least-significant) in each decoded field.
 *   For example, if decoding a 3D Morton code of width 30, this would be 10. While other Morton
 *   decoding libraries do not require this information, Morton ND uses it to perform
 *   compile-time optimizations, such as early termination.
 *
 *   WARNING: results will be incorrect if the decoder input width exceeds FieldBits * Dimensions.
 *
 * LutBits
 *   The LUT lookup width (in bits) must be provided as a template parameter. This is perhaps
 *   the most interesting parameter, since it allows the generated LUT and algorithm to be tuned
 *   for a particular CPU target and application.
 *
 *   This value dictates the size of the generated lookup table, as well as the number of lookups
 *   performed by the generated 'Decode' function.
 *
 *   LUT size in memory will be (appx.):    2^^LutBits * sizeof(std::array<LutValue, Dimensions>)
 *   Look-ups per Decode call:              ChunkCount
 *
 *   Note: ChunkCount and LutValue are available as static members for debugging.
 *
 *   To properly tune this value, consider the following:
 *
 *   - A larger value will result in an exponentially larger LUT and exponentially higher
 *     compilation times. For most use cases, 'LutBits' should not exceed 16.
 *   - A larger value will result in an 'Decode' function with fewer operations,
 *     iff it reduces ChunkCount (inspect this with the ChunkCount static member).
 *   - A 'Decode' function with minimal operations (see above) will not necessarily out-perform
 *     one that does more (smaller LUT) due to CPU caching. Smaller LUT configurations tend to do
 *     better for applications which call 'Decode' with random inputs.
 *
 *   For performance critical applications, run benchmarks.
 *
 * T
 *   The type of the Morton code to decode, as well as the element type of decoded fields in the
 *   returned std::tuple. This is optional (the fastest type which can fit the Morton code will be
 *   selected automatically), unless it would be too big to fit in a 64-bit unsigned integer.
 *
 *   If you need support for a result width > 64 but <= 128, you may be able to provide '__uint128_t'
 *   if your compiler supports it.
 *
 *   For > 128-bit results, a "BitInteger"-like class should work, but must support standard C++ integral
 *   operators. A std::size_t must also be constructible from this type (e.g. T provides an explicit
 *   conversion operator for std::size_t).
 *
 * @param Dimensions the number of fields (components) to decode.
 * @param FieldBits the number of bits in each input field, starting with the LSb.
 * @param LutBits the number of bits for the LUT.
 * @param T the type of the Morton code to decode, as well as the tuple types of the result.
 */
template<std::size_t Dimensions, std::size_t FieldBits, std::size_t LutBits, typename T = FastInt<Dimensions * FieldBits>>
class MortonNDLutDecoder
{
    constexpr static MortonNDLutValidator<Dimensions, FieldBits, LutBits, T> validate {};
    static constexpr auto MortonCodeWidth = FieldBits * Dimensions;
    static constexpr auto LutValueWidth = LutBits / Dimensions + (LutBits % Dimensions != 0);

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
    static constexpr std::size_t ChunkCount = MortonCodeWidth / LutBits + (MortonCodeWidth % LutBits != 0);

    /**
     * A mask which can be used to clear the upper bits of the input Morton code prior to
     * a call to 'Decode', if they're expected to be dirty.
     */
    constexpr T InputMask() const {
        static_assert(std::is_integral<T>::value, "Input masks are only provided for integral types.");
        return ~T(0) >> (std::numeric_limits<T>::digits - MortonCodeWidth);
    }

    /**
     * The type selected internally for LUT entry array values.
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

    /**
     * Decode a Morton code.
     * @param input The Morton code.
     * @return The decoded components of 'input' as an std::tuple.
     */
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
     *     its de-interleaved components. E.g.: LookupTable[yxzyxzyx] => [xxx, yyy, zz]
     *     - For each component,
     *       - Determine the destination field to which the component belongs:
     *         (ChunkStartBit + ComponentIndex) % Dimensions.
     *       - Inject the component's bits at the current write offset for the destination field:
     *         (ChunkStartBit + ComponentIndex) / Dimensions.
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
     * @tparam Args The type sequence corresponding to 'args'.
     * @param field The Morton code to decode.
     * @param args The tail of the chunk index sequence, used to pump the template.
     * @return The decoded components, as an 'std::tuple'.
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

    template <typename Array, std::size_t ...I>
    constexpr auto CreateTuple(Array arr, std::index_sequence<I...>) const {
        return std::make_tuple((T)arr[I]...);
    }

    constexpr auto DecodeInternal(T field, std::size_t) const
    {
        // This is the 0th chunk, so it lines up with the decode result array.
        return CreateTuple(LookupTable[field & ChunkMask], std::make_index_sequence<Dimensions>{});
    }

    template <size_t ...I>
    constexpr auto DecodeInternal(T field, std::index_sequence<I...>) const {
        return DecodeInternal(field, I ...);
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
