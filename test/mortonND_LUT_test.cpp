#include <morton-nd/mortonND_LUT_encoder.h>
#include <morton-nd/mortonND_LUT_decoder.h>
#include "mortonND_LUT_test.h"
#include "mortonND_test_common.h"
#include "variadic_placeholder.h"

template<size_t FieldBits, typename Ret, typename ...Fields, size_t ...FieldIdx, size_t ...N>
bool TestMortonNDLutSet(type_sequence<Fields...>, std::index_sequence<FieldIdx...>, std::index_sequence<N...>) {
    // Generate all LUTs at compile-time
    static constexpr auto luts = std::make_tuple(mortonnd::MortonNDLutEncoder<sizeof...(Fields), FieldBits, (N + 1)>()...);

    // Create function wrappers for each LUT's Encode instance method
    static const auto funcs = std::make_tuple(std::function<Ret(Fields...)>(
        std::bind(
            static_cast<Ret(std::tuple_element<N, decltype(luts)>::type::*)(Fields...) const>(&std::tuple_element<N, decltype(luts)>::type::Encode),
            std::get<N>(luts),
            variadic_placeholder<FieldIdx>{}...
        )
    )...); // expands N, generating a function per LUT

    return Reduce(std::logical_and<bool>{},
        (std::cout << "  Test: Bits/Lookup = " << N + 1, TestEncodeFunction<FieldBits>(std::get<N>(funcs)))...
    );
}

template<size_t Fields, size_t Digits, size_t FieldBits = Digits / Fields>
bool TestMortonNDLutSet() {
    std::cout << "Testing " << Digits << "-bit " << Fields << "D LUT encoders (Bits/Field = " << FieldBits << ")..." << std::endl;

    // (LUT size doesn't affect the type. 1 is arbitrary.)
    using T = typename mortonnd::MortonNDLutEncoder<Fields, FieldBits, 1>::type;
    return TestMortonNDLutSet<FieldBits, T>(
        make_type_sequence<Fields, T>(), std::make_index_sequence<Fields>{}, std::make_index_sequence<FieldBits>{});
}

bool mortonnd_lut::TestEncode() {
    return Reduce(
        std::logical_and<bool>{},

        // 1D
        TestMortonNDLutSet<1, 64, 18>(), // Limit FieldBits to 18
        TestMortonNDLutSet<1, 32, 18>(),
        TestMortonNDLutSet<1, 16>(),
        TestMortonNDLutSet<1, 8>(),

        // 2D
        TestMortonNDLutSet<2, 64, 18>(),
        TestMortonNDLutSet<2, 32>(),
        TestMortonNDLutSet<2, 16>(),
        TestMortonNDLutSet<2, 8>(),

        // 3D
        TestMortonNDLutSet<3, 64, 18>(),
        TestMortonNDLutSet<3, 32>(),
        TestMortonNDLutSet<3, 16>(),
        TestMortonNDLutSet<3, 8>(),

        // 4D
        TestMortonNDLutSet<4, 64>(),
        TestMortonNDLutSet<4, 32>(),
        TestMortonNDLutSet<4, 16>(),
        TestMortonNDLutSet<4, 8>(),

        // 5D
        TestMortonNDLutSet<5, 64>(),
        TestMortonNDLutSet<5, 32>(),
        TestMortonNDLutSet<5, 16>(),
        TestMortonNDLutSet<5, 8>(),

        // 8D
        TestMortonNDLutSet<8, 16>(),

        // 32 Dimensions
        TestMortonNDLutSet<32, 64>(),

        // 64 Dimensions
        TestMortonNDLutSet<64, 64>()
    );
}

template<size_t FieldBits, typename Ret, typename ...Fields, size_t ...FieldIdx, size_t ...N>
bool TestDecodeMortonNDLutSet(type_sequence<Fields...>, std::index_sequence<FieldIdx...>, std::index_sequence<N...>) {
    // Generate all LUTs at compile-time
    static constexpr auto luts = std::make_tuple(mortonnd::MortonNDLutDecoder<sizeof...(Fields), FieldBits, (N + 1)>()...);

    // Create function wrappers for each LUT's Encode instance method
    static const auto funcs = std::make_tuple(std::function<std::tuple<Fields...>(Ret)>(
        std::bind(
            static_cast<std::tuple<Fields...>(std::tuple_element<N, decltype(luts)>::type::*)(Ret) const>(&std::tuple_element<N, decltype(luts)>::type::Decode),
            std::get<N>(luts),
            std::placeholders::_1
        )
    )...); // expands N, generating a function per LUT

    return Reduce(std::logical_and<bool>{},
        (std::cout << "  Test: Bits/Lookup = " << (N + 1) << std::endl, TestDecodeFunction<FieldBits>(std::get<N>(funcs)))...
    );
}

template<size_t Fields, size_t Digits, size_t FieldBits = Digits / Fields>
bool TestDecodeMortonNDLutSet() {
    std::cout << "Testing " << Digits << "-bit " << Fields << "D LUT decoders (Bits/Field = " << FieldBits << ")..." << std::endl;

    // (LUT size doesn't affect the type. 1 is arbitrary.)
    using T = typename mortonnd::MortonNDLutDecoder<Fields, FieldBits, 1>::type;
    return TestDecodeMortonNDLutSet<FieldBits, T>(
        make_type_sequence<Fields, T>(), std::make_index_sequence<Fields>{}, std::make_index_sequence<FieldBits>{});
}

bool mortonnd_lut::TestDecode() {
    return Reduce(
        std::logical_and<bool>{},

        // 1D
        TestDecodeMortonNDLutSet<1, 64, 18>(), // Limit FieldBits to 18
        TestDecodeMortonNDLutSet<1, 32, 18>(),
        TestDecodeMortonNDLutSet<1, 16>(),
        TestDecodeMortonNDLutSet<1, 8>(),

        // 2D
        TestDecodeMortonNDLutSet<2, 64, 18>(),
        TestDecodeMortonNDLutSet<2, 32>(),
        TestDecodeMortonNDLutSet<2, 16>(),
        TestDecodeMortonNDLutSet<2, 8>(),

        // 3D
        TestDecodeMortonNDLutSet<3, 64, 18>(),
        TestDecodeMortonNDLutSet<3, 32>(),
        TestDecodeMortonNDLutSet<3, 16>(),
        TestDecodeMortonNDLutSet<3, 8>(),

        // 4D
        TestDecodeMortonNDLutSet<4, 64>(),
        TestDecodeMortonNDLutSet<4, 32>(),
        TestDecodeMortonNDLutSet<4, 16>(),
        TestDecodeMortonNDLutSet<4, 8>(),

        // 5D
        TestDecodeMortonNDLutSet<5, 64>(),
        TestDecodeMortonNDLutSet<5, 32>(),
        TestDecodeMortonNDLutSet<5, 16>(),
        TestDecodeMortonNDLutSet<5, 8>(),

        // 8D
        TestDecodeMortonNDLutSet<8, 16>(),

        // 32 Dimensions
        TestDecodeMortonNDLutSet<32, 64>(),

        // 64 Dimensions
        TestDecodeMortonNDLutSet<64, 64>()
    );
}