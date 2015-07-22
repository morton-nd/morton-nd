//
//  mortoncode.h
//  MetaMorton
//
//  Created by Kevin Hartman on 2/23/15.
//  Copyright (c) 2015 Kevin Hartman. All rights reserved.
//

#ifndef MetaMorton_mortoncode_h
#define MetaMorton_mortoncode_h

/**
 * @param bits the number of bits, starting with the LSB to include in the code
 * @param InputType the type of the components to encode/decode
 * @param OutputType the type of the code
 */
template<std::size_t N, std::size_t Bits, typename InputType, typename OutputType>
class MortonCode
{

public:
    OutputType Encode(InputType x, InputType y, InputType z)
    {


    }

private:
    static const InputType MaskTable[];
    static const InputType LookupTable[];

//    constexpr InputType Mask(int index) const {
//        return index == 0
//            ? ~(((InputType)-1) >> (sizeof(InputType) * 8 - Bits))
//            : Mask(index - 1)
//    }

    constexpr InputType Split1ByN(InputType component) const {

    }


};

#endif
