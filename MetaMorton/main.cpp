//
//  main.cpp
//  MetaMorton
//
//  Created by Kevin Hartman on 2/23/15.
//  Copyright (c) 2015 Kevin Hartman. All rights reserved.
//

#include <iostream>

#include "array_builder.h"

int main(int argc, const char * argv[]) {

    const int range = 256;

    for (int i = 0; i < range; i++) {
        printf("Value:%d\n", Array<range>::Value[i]);
    }


    return 0;
}
