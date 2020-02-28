# Morton ND
[![Build Status](https://travis-ci.org/kevinhartman/morton-nd.svg?branch=master)](https://travis-ci.org/kevinhartman/morton-nd) [![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://opensource.org/licenses/MIT)

A header-only Morton encode/decode library (C++14) capable of encoding from and decoding to N-dimensional space. Includes a hardware-based approach (using Intel BMI2) for newer Intel CPUs, as well as another fast approach based on the Lookup Table (LUT) method for other CPU variants. 

## Features
All algorithms are **generated** at compile-time for the number of dimensions and field width used. This way, loops and branches are not required.

### Encoding Support
- any number of dimensions (e.g. `2D, 3D, 4D ... ND`).
- built-in support for up to 128-bit native results (`__uint128_t`). Unlimited using a user-supplied "big integer" class.
- `constexpr` encode method, allowing Morton encodings to be expressed at compile-time.

### Decoder Support
- any number of dimensions (bounded by native result width of 64).
- built-in support for decoding up to 64-bit Morton codes.

## Encoders and Decoders

### Hardware (Intel BMI2)
Supports encoding and decoding in N dimensions, using Intel's BMI2 ISA extension (available in Haswell (Intel), Excavator (AMD) and newer).

See the [Morton ND BMI2 Usage Guide](docs/MortonND_BMI2.md) for details.

```c++
using MortonND_4D = mortonnd::MortonNDBmi<4, uint64_t>;

// Encodes 4 fields into a uint64_t result.
auto encoding = MortonND_4D::Encode(f1, f2, f3, f4);

// Decodes 4 fields.
std::tie(f1, f2, f3, f4) = MortonND_4D::Decode(encoding);
```

### Lookup Table (LUT)
Supports encoding in N dimensions, using a compiler-generated LUT-based approach.

LUTs are defined with constant expressions and thus can be generated (and even used) at compile-time.

The encoder supports chunking, allowing a LUT smaller than the input field width to be used internally. This is useful for applications which require faster compilation times and smaller binaries (at the expense of extra bit manipulation operations required to combine chunks at runtime).

See the [Morton ND LUT Usage Guide](docs/MortonND_LUT.md) for details.

```c++
// Generates a 4D LUT encoder (4 fields, 16 bits each, 8-bit LUT) using the compiler.
constexpr auto MortonND_4D = mortonnd::MortonNDLutEncoder<4, 16, 8>();

// Encodes 4 fields. Can be done at run-time or compile-time.
auto encoding = MortonND_4D.Encode(f1, f2, f3, f4);
```

## Testing and Performance
Validation testing specific to MortonND is located in the `tests` folder, covering N-Dimensional configurations where N ∈ { 1, 2, 3, 4, 5, 8, 16, 32, 64 } for common field sizes, and is run as part of Travis CI.

Performance benchmark tests (and additional validation) for 2D and 3D use cases are located in a separate repository. See [this fork](https://github.com/kevinhartman/libmorton#fork-changes) of @Forceflow's [Libmorton](https://github.com/Forceflow/libmorton), which integrates Morton ND into Libmorton's existing test framework.

### Benchmarks
The snippets below show performance comparisons between various 3D configurations of Morton ND. Comparisons to the 3D algorithms found in Libmorton are also included to demonstrate that Morton ND's generated algorithms are as efficient as hand-coded algorithms.

To run these tests (and more!) on your own machine, clone the fork linked above.

The following metrics (sorted by random access time, ascending) were collected on an i7-6920HQ, compiled with GCC 8.1 on macOS 10.13 using `-O3 -DNDEBUG`. Results include data from both linearly increasing and random inputs to demonstrate the performance impact of cache (hit or miss) under each algorithm / configuration. Results are averaged over 5 runs (each algorithm is run 5 times consecutively before moving on to the next).

#### 32-bit
```
++ Running each performance test 5 times and averaging results
++ Encoding 512^3 morton codes (134217728 in total)

    Linear      Random
    ======      ======
    644.383 ms  621.580 ms  : 32-bit (MortonND)    LUT: 1 chunks, 10 bit LUT
    648.476 ms  640.081 ms  : 32-bit (MortonND)    BMI2
    651.312 ms  643.003 ms  : 32-bit (lib-morton)  BMI2 instruction set
    740.515 ms  733.843 ms  : 32-bit (lib-morton)  LUT Shifted
    758.591 ms  747.817 ms  : 32-bit (lib-morton)  LUT
    788.011 ms  760.917 ms  : 32-bit (MortonND)    LUT: 2 chunks, 8 bit LUT
    815.661 ms  801.993 ms  : 32-bit (MortonND)    LUT: 2 chunks, 5 bit LUT
    1069.366 ms 1058.250 ms : 32-bit (lib-morton)  Magicbits
    1696.491 ms 1689.635 ms : 32-bit (lib-morton)  For ET
    1839.854 ms 1834.307 ms : 32-bit (lib-morton)  For
    
++ Decoding 512^3 morton codes (134217728 in total)

    Linear      Random
    ======      ======
    682.018 ms  3861.763 ms : 32-bit (lib-morton)  BMI2 Instruction set
    684.574 ms  3883.193 ms : 32-bit (MortonND)    MortonND: BMI2
    1034.161 ms 4241.783 ms : 32-bit (lib-morton)  LUT Shifted
    1145.437 ms 4372.682 ms : 32-bit (lib-morton)  Magicbits
    1192.125 ms 4375.222 ms : 32-bit (lib-morton)  LUT
    2435.355 ms 5892.361 ms : 32-bit (lib-morton)  For
    3441.597 ms 6166.849 ms : 32-bit (lib-morton)  For ET
```

#### 64-bit
```
++ Running each performance test 5 times and averaging results
++ Encoding 512^3 morton codes (134217728 in total)

    Linear      Random
    ======      ======
    682.781 ms  676.863 ms  : 64-bit (MortonND)    BMI2
    691.945 ms  687.623 ms  : 64-bit (lib-morton)  BMI2 instruction set
    984.001 ms  975.855 ms  : 64-bit (MortonND)    LUT: 3 chunks, 7 bit LUT
    762.784 ms  994.220 ms  : 64-bit (MortonND)    LUT: 2 chunks, 16 bit LUT
    996.148 ms  994.599 ms  : 64-bit (lib-morton)  LUT Shifted
    1067.235 ms 1064.724 ms : 64-bit (lib-morton)  LUT
    647.497 ms  1139.275 ms : 64-bit (MortonND)    LUT: 1 chunks, 21 bit LUT
    1287.558 ms 1284.486 ms : 64-bit (lib-morton)  Magicbits
    3695.027 ms 3691.027 ms : 64-bit (lib-morton)  For
    2366.875 ms 4015.031 ms : 64-bit (lib-morton)  For ET
    
++ Decoding 512^3 morton codes (134217728 in total)

    Linear      Random
    ======      ======
    679.403 ms  3819.525 ms : 64-bit (lib-morton)  BMI2 Instruction set
    689.146 ms  3863.148 ms : 64-bit (MortonND)    BMI2
    1338.503 ms 4600.688 ms : 64-bit (lib-morton)  Magicbits
    1481.526 ms 4672.497 ms : 64-bit (lib-morton)  LUT Shifted
    1748.424 ms 4974.474 ms : 64-bit (lib-morton)  LUT
    3108.945 ms 8864.368 ms : 64-bit (lib-morton)  For ET
    5764.171 ms 9157.460 ms : 64-bit (lib-morton)  For
```

## Thanks
* Jeroen Baert (@Forceflow)
  - [Morton encoding/decoding through bit interleaving: Implementations](https://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/)
  - [libmorton](https://github.com/Forceflow/libmorton), a C++11 header-only library for 2D and 3D Morton encoding and decoding.

## License
This project is licensed under the MIT license.

Attribution is appreciated where applicable, and as such, a NOTICE file is included which may be distributed in the credits of derivative software.
