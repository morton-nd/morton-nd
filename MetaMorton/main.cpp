//
//  main.cpp
//  MetaMorton
//
//  Created by Kevin Hartman on 2/23/15.
//  Copyright (c) 2015 Kevin Hartman. All rights reserved.
//

#include <iostream>

#include "mortoncode.h"

auto& morton = Morton<3, 4, uint32_t, uint32_t>;
const auto test = morton.Encode(5, 9, 1);

int main(int argc, const char * argv[]) {

    //MortonCode<3, 8, uint8_t, uint32_t> morton;
    //using MyMorton = Morton<3, 8, uint32_t, uint32_t>
    printf("Encoding of (1, 2, 3): %d\n", test);

    return 0;
}
