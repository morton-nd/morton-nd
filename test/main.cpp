#include <iostream>
#include <vector>

#include "mortonND_test.h"
#include "mortonND_LUT_test.h"
#include "mortonND_BMI2_test.h"

#include <iostream>

auto test_methods = std::vector<test_method>{
    test_method(&mortonnd_lut::TestEncode, "Test LUT encoder configurations (dimension, field size, LUT entry size)."),
    test_method(&mortonnd_bmi2::TestEncode, "Test BMI2 encoder configurations (dimension, field size)."),
    test_method(&mortonnd_bmi2::TestDecode, "Test BMI2 decoder configurations (dimension, field size)."),
    test_method(&mortonnd_lut::TestDecode, "Test LUT decoder configurations (dimension, field size, LUT entry size).")
};

int main(int argc, const char *argv[]) {
    bool ok = true;

    for (auto test : test_methods) {
        bool result = test.test_func();
        std::cout << (result ? "[passed]" : "[failed]") << "  " << test.description << std::endl;

        ok &= result;
    }

    return ok ? 0 : 1;
}
