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


template<class T, int Size>
union ArrayCombiner {
    const struct Inner {
        const std::array<T, Size> array1;
        const std::array<T, Size> array2;

        constexpr Inner(std::array<T, Size> arr1, std::array<T, Size> arr2) : array1(arr1), array2(arr2) { }
    } arrays;
    const std::array<T, Size * 2> combined;

    constexpr ArrayCombiner(std::array<T, Size> arr1, std::array<T, Size> arr2) : arrays(arr1, arr2) { }
};

template<int Size>
ArrayCombiner<int, (Size * 2)> constexpr GetN(int index) {
    return ArrayCombiner<int, Size * 2>(GetN<Size>(index).combined, GetN<Size>(index).combined);
    //return static_cast<std::array<int, 2>>((std::array<std::array<int, 2>, 2>){GetN(index), GetN(index)}) ;
}


const char storage[128];

constexpr std::array<int, 4> combined = ArrayCombiner<int, 2>({ 1, 2 }, { 1, 2 }).combined;



//std::array<int, 2> constexpr GetN(int index) {
//    int* test = &&GetN(index);
//    return static_cast<std::array<int, 2>>((std::array<std::array<int, 2>, 2>){GetN(index), GetN(index)}) ;
//}

//template<int N, int... Rest>
//struct Array {
//    static constexpr auto Value = Array<N - 1, N, Rest...>::Value;
//};
//
//template<int... Rest>
//struct Array<0, Rest...> {
//    static constexpr int Value[][2] = { GetN(0), GetN(Rest)... }; // TODO: could have N GetN(Rest)... functions that are called, but offset Rest by some amount. Not sure how to generate a collection of those calls.
//};
//
//template<int... Rest>
//constexpr int Array<0, Rest...>::Value[][2];


//std::array<int, 8> constexpr CombineArrays(const std::array<int, 4> one, const std::array<int, 4> two)
//{
//    std::
//}


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
    const uint Value = 1/*GetN(V)*/;
};


#endif
