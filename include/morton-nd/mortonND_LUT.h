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
}

#endif