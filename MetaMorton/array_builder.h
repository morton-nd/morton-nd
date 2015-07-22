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

#endif
