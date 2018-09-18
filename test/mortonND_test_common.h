#pragma once

#include <functional>
#include <tuple>
#include <iostream>

#include "mortonND_test_util.h"

template <typename T, size_t FieldBits, size_t SliceBits = FieldBits < 4 ? FieldBits : 4, typename Ret, typename ...Fields>
static bool TestEncodeFunction(const std::function<Ret(Fields...)>& function) {
	static const auto FieldCount = sizeof...(Fields);
	static_assert(FieldCount * FieldBits <= std::numeric_limits<uint64_t>::digits, "Control encoder cannot support > 64 bits.");
	static_assert(FieldBits >= SliceBits, "At least 4 bits from each field must fit into 'morton'");
	static_assert(std::numeric_limits<T>::digits >= FieldCount * FieldBits, "'morton' must support encoding width");

	// For every set of 4 contiguous bits, test all possible values (0-15), with all other bits cleared
	for (size_t offset = 0; offset <= FieldBits - SliceBits; offset++) {
		const std::function<bool(Fields...)> check_function = [&function, offset](Fields...fields) -> bool {
			const uint64_t correct = control_encode((fields << offset)...);
			const Ret computed = function((fields << offset)...);

			//PrintTuple(std::make_tuple((fields << offset)...), std::index_sequence_for<Fields...>{});

			if (computed != correct) {
				std::cout << "  Correct: " << correct << " Computed: " << computed << std::endl;
			}

			return computed == correct;
		};

		bool result = ApplyIndexSeqs<Pow(2, SliceBits), sizeof...(Fields)>(check_function, std::logical_and<bool>{});
		if (!result) return false;
	}

	return true;
}