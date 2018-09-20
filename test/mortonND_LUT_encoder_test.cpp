#include "mortonND_LUT_encoder_test.h"
#include "../morton-nd/include/mortonND_LUT_encoder.h"
#include "mortonND_test_control.h"
#include "mortonND_test_common.h"

template<size_t FieldBits, typename Ret, typename ...Fields, typename std::enable_if<sizeof...(Fields) < 6, int>::type = 0>
bool TestEncode(const std::function<Ret(Fields...)>& function) {
	return TestEncodeFunction<FieldBits>(function);
}

template<size_t FieldBits, typename Ret, typename ...Fields, typename std::enable_if<sizeof...(Fields) >= 6, int>::type = 0>
bool TestEncode(const std::function<Ret(Fields...)>& function) {
	std::cout << "  Using simple test." << std::endl;
	return TestEncodeFunctionSimple<FieldBits>(function);
}

template<size_t FieldBits, typename Ret, typename ...Fields, size_t ...FieldIdx, size_t ...N>
bool TestMortonNDLutSet(type_sequence<Fields...>, std::index_sequence<FieldIdx...>, std::index_sequence<N...>) {
	static const auto luts = std::make_tuple(mortonnd::MortonNDLutEncoder<sizeof...(Fields), FieldBits, (N + 1)>()...);
	static const auto funcs = std::make_tuple(
		std::function<Ret(Fields...)>(std::bind(static_cast<Ret(mortonnd::MortonNDLutEncoder<sizeof...(Fields), FieldBits, (N + 1)>::*)(Fields...) const>(&mortonnd::MortonNDLutEncoder<sizeof...(Fields), FieldBits, (N + 1)>::Encode), std::get<N>(luts), variadic_placeholder<FieldIdx>{}...))...
	);

	return Reduce(std::logical_and<bool>{}, 
		(std::cout << "Testing:  Fields = " << sizeof...(Fields) << ", Bits per field = " << FieldBits << ", Bits per LUT entry = " << (N + 1) << std::endl, TestEncode<FieldBits>(std::get<N>(funcs)))...
	);
}

template<size_t Fields, size_t Digits, size_t FieldBits = Digits / Fields>
bool TestMortonNDLutSet() {
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

		// 32 Dimensions
		TestMortonNDLutSet<32, 64>(),

		// 64 Dimensions
		TestMortonNDLutSet<64, 64>()
	);
}