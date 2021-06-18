//
//  mortonND_encoder.h
//  morton-nd
//
//  Copyright (c) 2015 Kevin Hartman.
//

#ifndef mortonND_h
#define mortonND_h

#include <cmath>
#include <array>
#include <tuple>
#include <type_traits>
#include <limits>

namespace mortonnd {

/**
 * Resolves to the narrowest native unsigned integer type that is at
 * least 'Bits' wide.
 *
 * If no native type can handle the requested width, this becomes type
 * 'void()', which will fail compilation when substituted later.
 *
 * This is used as the LutValue type to ensure that the lookup table
 * is as small as possible (and to improve cache hits in subsequent
 * lookups).
 *
 * Note:
 * For current use cases (i.e. as LutValue), a value 'Bits' > 64
 * will not be requested (it's guarded against by a static assertion).
 */
template<size_t Bits>
using MinInt =
    typename std::conditional<(Bits <= 8), uint_least8_t,
        typename std::conditional<(Bits <= 16), uint_least16_t,
            typename std::conditional<(Bits <= 32), uint_least32_t,
                typename std::conditional<(Bits <= 64), uint_least64_t, void()>
                    ::type>::type>::type>::type;

/**
 * Resolves to the fastest *non-implicitly-promotable* native unsigned
 * integer type that is at least 'Bits' wide, or 'Default' if no such
 * type exists.
 *
 * This is used to select the interface type (T) used for encoding
 * inputs as well as the result.
 *
 * Note:
 * It's important that this type is not implicitly promotable to a
 * *signed* integer type, which could produce incorrect results.
 *
 * Signed types could be supported with input masking, but this would
 * add extra overhead.
 */
template<size_t Bits, typename Default = void()>
using FastInt =
    typename std::conditional<(Bits <= 32), uint_fast32_t,
        typename std::conditional<(Bits <= 64), uint_fast64_t, Default>
            ::type>::type;


template<std::size_t Dimensions, std::size_t FieldBits, std::size_t LutBits, typename T = FastInt<Dimensions * FieldBits>>
struct MortonNDLutValidator
{
    static constexpr auto LutValueWidth = LutBits * Dimensions;

    static_assert(Dimensions > 0, "'Dimensions' must be > 0.");
    static_assert(FieldBits > 0, "'FieldBits' must be > 0. ");
    static_assert(LutBits > 0, "'LutBits' must be > 0.");
    static_assert(LutBits <= FieldBits, "'LutBits' must be <= 'FieldBits'.");

    // Note: there's no technical reason for '32', but a larger value would be unreasonable.
    static_assert(LutBits <= 32, "'LutBits' must be <= 32.");

    static_assert(LutValueWidth <= 64, "'LutBits' * 'Dimensions' must be <= 64.");
    static_assert(LutValueWidth <= std::numeric_limits<std::size_t>::digits,
        "'LutBits' * 'Dimensions' must be <= width of std::size_t.");

    static_assert(!std::is_integral<T>::value || (std::numeric_limits<T>::digits >= (Dimensions * FieldBits)),
        "'T' must be able to hold 'Dimensions' * 'FieldBits' bits (the result size).");

    static_assert(!std::is_integral<T>::value || std::is_unsigned<T>::value,
        "'T' must be unsigned.");

    // LUT lookups require conversion from T to std::size_t.
    // Note: this does not imply that T must fit within std::size_t.
    static_assert(std::is_constructible<std::size_t, T>::value, "std::size_t must be constructible from 'T'");
};
}

#endif