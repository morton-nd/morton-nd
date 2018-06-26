//
//  main.cpp
//  MetaMorton
//
//  Created by Kevin Hartman on 2/23/15.
//  Copyright (c) 2015 Kevin Hartman. All rights reserved.
//

#include <iostream>

#include "../morton-nd/include/mortonND_encoder.h"

constexpr auto Morton = mortonnd::MortonNDEncoder<3, 1, 17>();

const auto test = Morton.Encode(5, 9, 1);

int main(int argc, const char * argv[]) {
    printf("Encoding of (5, 9, 1): %d\n", test);

    for (int i = 0; i < 50; ++i) {
        printf("Morton Encoding of (%d %d %d): %d\n", i, i+1, i+2, Morton.Encode(i, i + 1, i + 2));
    }

    return 0;
}
