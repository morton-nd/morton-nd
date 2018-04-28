//
//  mortoncode.h
//  MetaMorton
//
//  Created by Kevin Hartman on 2/23/15.
//  Copyright (c) 2015 Kevin Hartman. All rights reserved.
//

#ifndef MetaMorton_mortoncode_h
#define MetaMorton_mortoncode_h

#include <cmath>
#include <array>

/**
 * @param Bits the number of bits, starting with the LSB to include in the code
 * @param InputType the type of the components to encode/decode
 * @param OutputType the type of the code
 */
template<std::size_t Fields, std::size_t Bits, typename InputType, typename OutputType>
class MortonCode
{

public:
    constexpr MortonCode(): LookupTable(BuildLut(std::make_index_sequence<LutSize()>{})) {}
    
    OutputType Encode(InputType x, InputType y, InputType z)
    {


    }
    
    static constexpr InputType Split1ByN(InputType input, size_t bitsRemaining = Bits) {
        static_assert(Fields > 0, "Field parameter (# fields) must be > 0");
        
        return (bitsRemaining == 0) ? input : (Split1ByN(input >> 1, bitsRemaining - 1) << Fields) | (input & 1);
    }
    
    template<size_t... i>
    static constexpr auto BuildLut(std::index_sequence<i...>) {
        return std::array<InputType, sizeof...(i)>{{Split1ByN(i)...}};
    }
    
    static constexpr size_t pow(size_t base, size_t exp) {
        return exp == 0 ? 1 : base * pow(base, exp - 1);
    }

    static constexpr size_t LutSize() {
        return pow(2, Bits);
    }

//private:
    const std::array<InputType, LutSize()> LookupTable;

//    constexpr InputType Mask(int index) const {
//        return index == 0
//            ? ~(((InputType)-1) >> (sizeof(InputType) * 8 - Bits))
//            : Mask(index - 1)
//    }

    


};

template<std::size_t Fields, std::size_t Bits, typename InputType, typename OutputType>
constexpr auto Morton = MortonCode<Fields, Bits, InputType, OutputType>();

#endif
