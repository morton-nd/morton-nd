//
//  main.cpp
//  MetaMorton
//
//  Created by Kevin Hartman on 2/23/15.
//  Copyright (c) 2015 Kevin Hartman. All rights reserved.
//

#include <iostream>

#include "mortoncode.h"

int main(int argc, const char * argv[]) {

//    const int range = 256;
//
//    for (int i = 0; i < range; i++) {
//        printf("Value:%d\n", Array<range>::Value[i]);
//    }

    auto& morton = Morton<3, 8, uint32_t, uint32_t>;

    //MortonCode<3, 8, uint8_t, uint32_t> morton;
    //using MyMorton = Morton<3, 8, uint32_t, uint32_t>;
    
    for (int i = 0; i < 256; i++) {
        uint32_t split = morton.LookupTable[i];
        
        //Member<6, 1 << 6> test;
        //printf("Value:%d\n", test.array[1]);
        printf("Value:0x%2x\n", split);
    }
    return 0;
}
