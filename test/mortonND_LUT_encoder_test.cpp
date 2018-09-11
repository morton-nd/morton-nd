#include "mortonND_LUT_encoder_test.h"
#include "../morton-nd/include/mortonND_LUT_encoder.h"
#include "mortonND_test_control.h"
#include "mortonND_test_common.h"

//constexpr auto Morton_1_12 = mortonnd::MortonNDLutEncoder<3, 12, 12>();
constexpr auto Morton_2_6 = mortonnd::MortonNDLutEncoder<3, 12, 6>();
constexpr auto Morton_3_4 = mortonnd::MortonNDLutEncoder<3, 12, 4>();
constexpr auto Morton_4_3 = mortonnd::MortonNDLutEncoder<3, 12, 3>();
constexpr auto Morton_6_2 = mortonnd::MortonNDLutEncoder<3, 12, 2>();
constexpr auto Morton_12_1 = mortonnd::MortonNDLutEncoder<3, 12, 1>();

constexpr uint64_t Morton_2_6_Encode(uint64_t x, uint64_t y, uint64_t z) {
	return Morton_2_6.Encode(x, y, z);
}

int mortonnd_lut::TestEncode() {
	std::function<uint64_t(uint64_t, uint64_t, uint64_t)> f = &Morton_2_6_Encode;
	return check2D_EncodeFunction<uint64_t, 12>(f);

	/*for (uint64_t i = 0; i < 16; i++) {
		for (uint64_t j = 0; j < 16; j++) {
			for (uint64_t k = 0; k < 16; k++) {
				auto control = control_encode(i, j, k);
					if (Morton_2_6.Encode(i, j, k) != control ||
						Morton_3_4.Encode(i, j, k) != control ||
						Morton_4_3.Encode(i, j, k) != control ||
						Morton_6_2.Encode(i, j, k) != control ||
						Morton_12_1.Encode(i, j, k) != control) 
						return -1;
			}
		}
	}*/

	return 0;
}