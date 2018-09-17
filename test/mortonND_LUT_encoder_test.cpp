#include "mortonND_LUT_encoder_test.h"
#include "../morton-nd/include/mortonND_LUT_encoder.h"
#include "mortonND_test_control.h"
#include "mortonND_test_common.h"

constexpr auto Morton_1_12 = mortonnd::MortonNDLutEncoder<3, 12, 12>();
constexpr auto Morton_2_6 = mortonnd::MortonNDLutEncoder<3, 12, 6>();
constexpr auto Morton_3_4 = mortonnd::MortonNDLutEncoder<3, 12, 4>();
constexpr auto Morton_4_3 = mortonnd::MortonNDLutEncoder<3, 12, 3>();
constexpr auto Morton_6_2 = mortonnd::MortonNDLutEncoder<3, 12, 2>();
constexpr auto Morton_12_1 = mortonnd::MortonNDLutEncoder<3, 12, 1>();

template<size_t FieldBits, typename Ret, typename ...Fields, size_t ...N, size_t ...FieldIdx>
bool TestMortonNDLutSet(std::index_sequence<N...>, std::index_sequence<FieldIdx...>) {
	static const auto luts = std::make_tuple(mortonnd::MortonNDLutEncoder<sizeof...(Fields), FieldBits, (N + 1)>()...);
	static const auto funcs = std::make_tuple(
		std::function<Ret(Fields...)>(std::bind(static_cast<Ret(mortonnd::MortonNDLutEncoder<sizeof...(Fields), FieldBits, (N + 1)>::*)(Fields...) const>(&mortonnd::MortonNDLutEncoder<sizeof...(Fields), FieldBits, (N + 1)>::Encode), std::get<N>(luts), variadic_placeholder<FieldIdx>{}...))...
	);
	return Reduce(std::logical_and<bool>{}, 
		(std::cout << "Testing:  Fields = " << sizeof...(Fields) << ", Bits per field = " << FieldBits << ", Bits per LUT entry = " << (N + 1) << std::endl, TestEncodeFunction<Ret, FieldBits>(std::get<N>(funcs)))...
	);
}

constexpr uint64_t Morton_2_6_Encode(uint64_t x, uint64_t y, uint64_t z) {
	return Morton_2_6.Encode(x, y, z);
}

bool mortonnd_lut::TestEncode() {
	//std::function<uint64_t(uint64_t, uint64_t, uint64_t)> f = &Morton_2_6_Encode;
	//return TestEncodeFunction<uint64_t, 12>(f);
	return TestMortonNDLutSet<18, uint64_t, uint64_t, uint64_t, uint64_t>(std::make_index_sequence<18>{}, std::make_index_sequence<3>{});
}