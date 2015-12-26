#include <array>
//
//  array_builder.h
//  MetaMorton
//
//  Created by Kevin Hartman on 2/26/15.
//  Copyright (c) 2015 Kevin Hartman. All rights reserved.
//

#ifndef MetaMorton_array_builder_h
#define MetaMorton_array_builder_h

int constexpr GetN(int index) {
    return index;
}

template<int N, int... Rest>
struct Array {
    static constexpr auto Value = Array<N - 1, N, Rest...>::Value;
};

template<int... Rest>
struct Array<0, Rest...> {
    static constexpr int Value[] = { GetN(0), GetN(Rest)... };
};

template<int... Rest>
constexpr int Array<0, Rest...>::Value[];




template<int N, int V>
struct Member {
    struct Values {
        const Member<(N - 1), V - (1 << (N - 1))> v1;
        const Member<(N - 1), V> v2;

        Values() : v1(), v2() {}
    };

    const union {
        const Values values;
        const std::array<uint, 1 << N> array;
    };

    Member() : values() { }
};

template<int V>
struct Member<0, V> {
    const uint Value = GetN(V);
};


#endif
