#pragma once

#include <functional>
#include <tuple>
#include <iostream>

#include "mortonND_test_util.h"
#include "mortonND_test_control.h"

template<size_t FieldBits, typename Ret, typename ...Fields>
static bool TestEncodeFunction_Fast(const std::function<Ret(Fields...)> &function) {
    static const auto MaxValue = Pow(2, FieldBits);
    static const auto FieldCount = sizeof...(Fields);
    static_assert(FieldCount * FieldBits <= std::numeric_limits<uint64_t>::digits, "Control encoder cannot support > 64 bits.");
    static_assert(std::numeric_limits<Ret>::digits >= FieldCount * FieldBits, "'morton' must support encoding width");

    bool ok = true;

    const std::function<bool(Fields...)> check_function = [&function](auto...fields) -> bool {
        const uint64_t correct = control_encode<Fields...>(fields...);
        const Ret computed = function(fields...);

        if (computed != correct) {
            std::cout << "  Mismatch when encoding";
            PrintTuple(std::make_tuple(fields...), std::index_sequence_for<Fields...>{});
            std::cout << "    Correct: " << correct << " Computed: " << computed << std::endl;
        }

        return computed == correct;
    };

    for (size_t i; i < MaxValue; i++) {
        ok &= check_function(static_cast<Fields>(i)...);
    }

    return ok;
};

template<size_t FieldBits, size_t SliceBits = FieldBits < 4 ? FieldBits : 4, typename Ret, typename ...Fields>
static bool TestEncodeFunction_Perms(const std::function<Ret(Fields...)> &function) {
    static const auto FieldCount = sizeof...(Fields);
    static_assert(FieldCount * FieldBits <= std::numeric_limits<uint64_t>::digits, "Control encoder cannot support > 64 bits.");
    static_assert(FieldBits >= SliceBits, "At least 4 bits from each field must fit into 'morton'");
    static_assert(std::numeric_limits<Ret>::digits >= FieldCount * FieldBits, "'Ret' must support encoding width");

    bool ok = true;

    // For every set of 4 contiguous bits, test all possible values (0-15), with all other bits cleared
    for (size_t offset = 0; offset <= FieldBits - SliceBits; offset++) {
        const std::function<bool(Fields...)> check_function = [&function, offset](auto...fields) -> bool {
            const uint64_t correct = control_encode<Fields...>((fields << offset)...);
            const Ret computed = function((fields << offset)...);

            if (computed != correct) {
                std::cout << "  Mismatch when encoding";
                PrintTuple(std::make_tuple((fields << offset)...), std::index_sequence_for<Fields...>{});
                std::cout << "    Correct: " << correct << " Computed: " << computed << std::endl;
            }

            return computed == correct;
        };

        ok &= ApplyIndexSeqs<Ret, Pow(2, SliceBits), sizeof...(Fields)>(check_function, std::logical_and<bool>{});
    }

    return ok;
}


template<size_t FieldBits, typename Ret, typename ...Fields, typename std::enable_if<sizeof...(Fields) < 6, int>::type = 0>
static bool TestEncodeFunction(const std::function<Ret(Fields...)> &function) {
    std::cout << std::endl;
    return TestEncodeFunction_Perms<FieldBits>(function);
}

template<size_t FieldBits, typename Ret, typename ...Fields, typename std::enable_if<sizeof...(Fields) >= 6, int>::type = 0>
static bool TestEncodeFunction(const std::function<Ret(Fields...)> &function) {
    std::cout << " (Dimensions > 5. Falling back to simple test)" << std::endl;
    return TestEncodeFunction_Fast<FieldBits>(function);
}

template<typename T> struct TD;

template<size_t FieldBits, size_t SliceBits = FieldBits < 4 ? FieldBits : 4, typename Ret, typename ...Fields>
static bool TestDecodeFunction(const std::function<std::tuple<Fields...>(Ret)> &function) {
    static const auto FieldCount = sizeof...(Fields);
    static const auto EncodingBits = FieldCount * FieldBits;
    static_assert(EncodingBits <= std::numeric_limits<uint64_t>::digits, "Control decoder cannot support > 64 bits.");
    static_assert(std::numeric_limits<Ret>::digits >= EncodingBits, "'Ret' must support encoding width");

    bool ok = true;

    std::tuple<std::conditional_t<true, uint64_t, Fields>...> ctrl_decoding;
    auto ctrl_ref_tup = RefTuple(ctrl_decoding);

    for (size_t offset = 0; offset < (EncodingBits - FieldCount * SliceBits); offset++) {
        for (Ret i = 0; i < Pow(2, SliceBits); i++) {
            Ret encoding = i << offset;

            // Concat encoding and ctrl_decoding references into parameter list and call control decoder
            Apply(&control_decode<std::conditional_t<true, uint64_t, Fields>...>, std::tuple_cat(std::make_tuple(encoding), ctrl_ref_tup));

            const auto decoding = function(encoding);

            // TODO: error messages don't specify which encoding, or which field from that encoding are to blame
            const auto compare = [](uint64_t control_field, Ret actual_field) -> bool {
                if (actual_field != control_field) {
                    std::cout << "  Mismatch when decoding";
                    std::cout << "    Correct field: " << control_field << " Computed field: " << actual_field << std::endl;

                    return false;
                }
                return true;
            };

            const auto zipped = Zip(ctrl_decoding, decoding);
            const auto results = ApplyEach(compare, zipped);

            ok &= Apply([](auto ...params) { return Reduce(std::logical_and<bool>{}, params...); }, results);
        }
    }

    return ok;
};