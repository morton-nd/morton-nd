#pragma once

#include <functional>
#include <tuple>
#include <experimental/tuple>

template<typename Ret, typename ...Fields>
//using encode_func = Ret(*)(Fields...);
using encode_func = std::function<Ret(Fields...)>;

static constexpr size_t pow(size_t base, size_t exp) {
	return exp == 0 ? 1 : base * pow(base, exp - 1);
}

//template<typename Ret, typename ...Fields, size_t ...I>
//static bool Loop(encode_func<Ret, Fields...> function, std::index_sequence<I...>) {
//	//bool results[sizeof...(Fields)] = { ((Fields*)0, function(field...))... };
//	
//	bool results[sizeof...(Fields) * sizeof...(I)] = { (function((Fields)I))...... };
//
//	for (int i = 0; i < sizeof...(Fields) * sizeof...(I); i++) {
//		if (results[i] == false) return false;
//	}
//
//	return true;
//}

template<typename Ret, typename ...OutFields, typename InField, size_t ...i>
static bool Loop2(std::tuple<OutFields...> out, std::tuple<InField> in, encode_func<Ret, OutFields..., InField> function, std::index_sequence<i...>) {
	bool results[16] = { std::experimental::apply(function, std::tuple_cat(out, std::make_tuple(i)))... };

	for (int index = 0; index < 16; index++) {
		if (!results[index]) return false;
	}

	return true;
}

template<typename Ret, typename ...OutFields, typename InField, typename ...InFields, size_t ...i>
static bool Loop2(std::tuple<OutFields...> out, std::tuple<InField, InFields...> in, encode_func<Ret, OutFields..., InField, InFields...> function, std::index_sequence<i...>) {
	bool results[16] = { Loop2(std::tuple_cat(out, std::make_tuple(i)), std::make_tuple(((InFields)0)...), function, std::make_index_sequence<16>{})... };

	for (int index = 0; index < 16; index++) {
		if (!results[index]) return false;
	}

	return true;
}

template <typename T, size_t FieldBits, typename Ret, typename ...Fields>
static bool check2D_EncodeFunction(encode_func<Ret, Fields...> function) {
	static const auto FieldCount = sizeof...(Fields);
	static_assert(FieldCount * FieldBits <= std::numeric_limits<uint64_t>::digits, "Control encoder cannot support > 64 bits.");
	static_assert(FieldBits >= 4, "At least 4 bits from each field must fit into 'morton'");
	static_assert(std::numeric_limits<T>::digits >= FieldCount * FieldBits, "'morton' must support encoding width");

	bool everything_okay = true;
	T computed_code;
	uint64_t correct_code = 0;

	return Loop2(std::make_tuple<>(), std::make_tuple(((Fields)0)...), function, std::make_index_sequence<16>{});

	// For every set of 4 contiguous bits, test all possible values (0-15), with all other bits cleared
	/*for (size_t offset = 0; offset <= FieldBits - 4; offset++) {
		for (coord i = 0; i < 16; i++) {
			for (coord j = 0; j < 16; j++) {
				coord x = i << offset;
				coord y = j << offset;

				correct_code = control_encode(x, y);
				computed_code = function.encode(x, y);
				if (computed_code != (T)correct_code) {
					everything_okay = false;
					cout << endl << "    Incorrect encoding of (" << x << ", " << y << ") in method " << function.description.c_str() << ": " << computed_code <<
						" != " << (T)correct_code << endl;
				}
			}
		}
	}
	return everything_okay;*/
}