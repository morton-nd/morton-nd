//
//  main.cpp
//  MetaMorton
//
//  Created by Kevin Hartman on 2/23/15.
//  Copyright (c) 2015 Kevin Hartman. All rights reserved.
//

#include <iostream>
#include <valarray>

#include "../morton-nd/include/mortonND_encoder.h"

uint64_t split_by_n(uint64_t input, size_t fieldCount, size_t bitsRemaining) {
    return (bitsRemaining == 0) ? input : (split_by_n(input >> 1, fieldCount, bitsRemaining - 1) << fieldCount) | (input & 1);
}

uint64_t control_encode(size_t bitCount, const std::valarray<uint64_t>& fields, const size_t fieldCount) {
    // TODO: return if fields is 1
    if (fields.size() == 1) {
        return split_by_n(fields[std::slice(0, 1, 1)][0], fieldCount, bitCount);
    }

    return (control_encode(bitCount, fields[std::slice(1, fields.size() - 1, 1)], fieldCount) << 1) | split_by_n(fields[std::slice(0, 1, 1)][0], fieldCount, bitCount);
}

/**
 * Runtime Morton encoding using 64-bit fields. bitCount * |fields| must be <64.
 *
 * @param bitCount The number of bits (starting with LSb) in each field.
 * @param fields The number of fields to encode.
 * @return The Morton encoding.
 */
uint64_t control_encode(size_t bitCount, const std::valarray<uint64_t>& fields) {
    return control_encode(bitCount, fields, fields.size());
}

constexpr auto Morton_1_12 = mortonnd::MortonNDEncoder<3, 1, 12>();
constexpr auto Morton_2_6 = mortonnd::MortonNDEncoder<3, 2, 6>();
constexpr auto Morton_3_4 = mortonnd::MortonNDEncoder<3, 3, 4>();
constexpr auto Morton_4_3 = mortonnd::MortonNDEncoder<3, 4, 3>();
constexpr auto Morton_6_2 = mortonnd::MortonNDEncoder<3, 6, 2>();
constexpr auto Morton_12_1 = mortonnd::MortonNDEncoder<3, 12, 1>();

void test_16_16_16() {
    for (uint64_t i = 0; i < 16; i++) {
        for (uint64_t j = 0; j < 16; j++) {
            for (uint64_t k = 0; k < 16; k++) {
                printf("(%d, %d, %d, %d, %d, %d, %d)\n",
                       control_encode(12, {i, j, k}),
                       Morton_1_12.Encode(i, j, k),
                       Morton_2_6.Encode(i, j, k),
                       Morton_3_4.Encode(i, j, k),
                       Morton_4_3.Encode(i, j, k),
                       Morton_6_2.Encode(i, j, k),
                       Morton_12_1.Encode(i, j, k)
                );
            }
        }
    }
}


int main(int argc, const char * argv[]) {
    test_16_16_16();

    return 0;
}
