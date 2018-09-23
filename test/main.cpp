//
//  main.cpp
//  MetaMorton
//
//  Created by Kevin Hartman on 2/23/15.
//  Copyright (c) 2015 Kevin Hartman. All rights reserved.
//

#include <iostream>
#include <vector>

#include "mortonND_test.h"
#include "mortonND_test_control.h"
#include "mortonND_LUT_encoder_test.h"

#include <iostream>

auto test_methods = std::vector<test_method>{
	test_method(&mortonnd_lut::TestEncode, "Test LUT encoder configurations (dimension, field size, LUT entry size.")
};

int main(int argc, const char * argv[]) {
	for (auto test : test_methods) {
		bool result = test.test_func();
		std::cout << (result ? "[passed]" : "[failed]") << "  " << test.description << std::endl;
	}

    return 0;
}
