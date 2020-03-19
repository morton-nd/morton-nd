#include "mortonND_BMI2_test.h"
#include "mortonND_test_util.h"
#include "mortonND_test_common.h"

#include <morton-nd/mortonND_BMI2.h>

template<typename Ret, typename ...Fields>
bool TestMortonNDBmiEncoder(type_sequence<Fields...>) {
    static const auto func = std::function<Ret(Fields...)>(static_cast<Ret(*)(Fields...)>(mortonnd::MortonNDBmi<sizeof...(Fields), Ret>::Encode));
    return TestEncodeFunction<std::numeric_limits<Ret>::digits / sizeof...(Fields)>(func);
}

template<size_t Fields, typename T>
bool TestMortonNDBmiEncoder() {
    std::cout << "Testing " << std::numeric_limits<T>::digits << "-bit " << Fields << "D BMI2 encoders..." << std::endl;
    return TestMortonNDBmiEncoder<T>(make_type_sequence<Fields, T>());
}

template<typename Ret, typename ...Fields>
bool TestMortonNDBmiDecoder(type_sequence<Fields...>) {
    static const auto func = std::function<std::tuple<Fields...>(Ret)>(static_cast<std::tuple<Fields...>(*)(Ret)>(mortonnd::MortonNDBmi<sizeof...(Fields), Ret>::Decode));
    return TestDecodeFunction<std::numeric_limits<Ret>::digits / sizeof...(Fields)>(func);
}

template<size_t Fields, typename T>
bool TestMortonNDBmiDecoder() {
    std::cout << "Testing " << std::numeric_limits<T>::digits << "-bit " << Fields << "D BMI2 decoders..." << std::endl;
    return TestMortonNDBmiDecoder<T>(make_type_sequence<Fields, T>());
}

bool mortonnd_bmi2::TestEncode() {
    return Reduce(std::logical_and<bool>{},
        TestMortonNDBmiEncoder<1, uint64_t>(),
        TestMortonNDBmiEncoder<1, uint32_t>(),

        TestMortonNDBmiEncoder<2, uint64_t>(),
        TestMortonNDBmiEncoder<2, uint32_t>(),

        TestMortonNDBmiEncoder<3, uint64_t>(),
        TestMortonNDBmiEncoder<3, uint32_t>(),

        TestMortonNDBmiEncoder<4, uint64_t>(),
        TestMortonNDBmiEncoder<4, uint32_t>(),

        TestMortonNDBmiEncoder<5, uint64_t>(),
        TestMortonNDBmiEncoder<5, uint32_t>(),

        TestMortonNDBmiEncoder<8, uint64_t>(),
        TestMortonNDBmiEncoder<8, uint32_t>(),

        TestMortonNDBmiEncoder<16, uint64_t>(),
        TestMortonNDBmiEncoder<16, uint32_t>(),

        TestMortonNDBmiEncoder<32, uint64_t>(),
        TestMortonNDBmiEncoder<32, uint32_t>(),

        TestMortonNDBmiEncoder<64, uint64_t>()
    );
}

bool mortonnd_bmi2::TestDecode() {
    return Reduce(std::logical_and<bool>{},
        TestMortonNDBmiDecoder<1, uint32_t>(),
        TestMortonNDBmiDecoder<2, uint64_t>(),
        TestMortonNDBmiDecoder<2, uint32_t>(),

        TestMortonNDBmiDecoder<3, uint64_t>(),
        TestMortonNDBmiDecoder<3, uint32_t>(),

        TestMortonNDBmiDecoder<4, uint64_t>(),
        TestMortonNDBmiDecoder<4, uint32_t>(),

        TestMortonNDBmiDecoder<5, uint64_t>(),
        TestMortonNDBmiDecoder<5, uint32_t>(),

        TestMortonNDBmiDecoder<8, uint64_t>(),
        TestMortonNDBmiDecoder<8, uint32_t>(),

        TestMortonNDBmiDecoder<16, uint64_t>(),
        TestMortonNDBmiDecoder<16, uint32_t>(),

        TestMortonNDBmiDecoder<32, uint64_t>(),
        TestMortonNDBmiDecoder<32, uint32_t>(),

        TestMortonNDBmiDecoder<64, uint64_t>()
    );
}