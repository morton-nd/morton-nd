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