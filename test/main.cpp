#include <iostream>
#include <vector>

#include "mortonND_test.h"
#include "mortonND_LUT_encoder_test.h"

#include <iostream>

auto test_methods = std::vector<test_method>{
    test_method(&mortonnd_lut::TestEncode, "Test LUT encoder configurations (dimension, field size, LUT entry size.")
};

int main(int argc, const char *argv[]) {
    for (auto test : test_methods) {
        bool result = test.test_func();
        std::cout << (result ? "[passed]" : "[failed]") << "  " << test.description << std::endl;
    }

    return 0;
}
