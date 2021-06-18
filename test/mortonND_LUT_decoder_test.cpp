#include <morton-nd/mortonND_LUT_decoder.h>
#include <morton-nd/mortonND_BMI2.h>
#include "mortonND_LUT_decoder_test.h"
#include "mortonND_test_common.h"
#include "variadic_placeholder.h"

constexpr auto Dimensions = 3;
constexpr auto LutBits = 9;
constexpr auto FieldBits = 9;
constexpr auto decoder = mortonnd::MortonNDLutDecoder<Dimensions, FieldBits, LutBits>();
constexpr auto Input = 3452356;

bool mortonnd_lut::TestDecode() {
    auto decoder = mortonnd::MortonNDLutDecoder<Dimensions, FieldBits, LutBits>();
    auto mask = mortonnd::MortonNDLutDecoder<2, 8, 2>::ChunkMask;

    auto decoded = decoder.Decode(Input & decoder.InputMask());

    std::cout << "LUT [";
    for (auto& field : decoded) {
        std::cout << static_cast<unsigned>(field) << ", ";
    }
    std::cout << "]";

    std::cout << std::endl;

    auto bmiDecode = mortonnd::MortonNDBmi<Dimensions, uint64_t>::Decode(Input);
    std::cout << "BMI [";
    PrintTuple(bmiDecode, std::make_index_sequence<Dimensions> {});
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