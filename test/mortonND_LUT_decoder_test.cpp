#include <morton-nd/mortonND_LUT_decoder.h>
#include <morton-nd/mortonND_BMI2.h>
#include "mortonND_LUT_decoder_test.h"
#include "mortonND_test_common.h"
#include "variadic_placeholder.h"

bool mortonnd_lut::TestDecode() {
    auto decoder = mortonnd::MortonNDLutDecoder<2, 8, 8>();

    auto decoded = decoder.Decode(0b101010);

    std::cout << "LUT [";
    for (auto& field : decoded) {
        std::cout << static_cast<unsigned>(field) << ", ";
    }
    std::cout << "]";

    std::cout << std::endl;

    auto bmiDecode = mortonnd::MortonNDBmi<2, uint64_t>::Decode(0b101010);
    std::cout << "BMI [" << std::get<0>(bmiDecode) << ", " << std::get<1>(bmiDecode) << ", " << "]";
    std::cout << std::endl;

    std::size_t index = 0;
    for (auto& row : decoder.LookupTable) {
        std::cout << index++ << ": [";

        for (auto item : row) {
            std::cout << static_cast<unsigned>(item) << ", ";
        }
        std::cout << "]" << std::endl;
    }

    //std::cout << mortonnd::JoinByN<8>(21, 2);

    return true;
}