# Morton ND
A C++14 header-only Morton encoding library for N dimensions. Includes a hardware-based approach (using Intel BMI2) for newer Intel CPUs, as well as another fast approach based on the Lookup Table (LUT) method for other CPU variants. 

### Hardware (Intel BMI2)
Supports encoding and decoding in N dimensions, using Intel's BMI2 ISA extension (available in Haswell (Intel), Excavator (AMD) and newer).

See the [Morton ND BMI2 Usage Guide](docs/MortonND_BMI2.md) for details.

```c++
// Encodes 4 fields into a uint64_t result.
auto encoding = mortonnd::MortonNDBmiEncoder<4, uint64_t>().Encode(f1, f2, f3, f4);
```

### Lookup Table (LUT)
Supports encoding only in N dimensions, using a compiler-generated LUT-based approach.

LUTs are defined with constant expressions and thus can be generated (and even used) at compile-time.

The encoder supports chunking, allowing a LUT smaller than the input field width to be used internally. This is useful for applications which require faster compilation times and smaller binaries (at the expense of extra bit manipulation operations required to combine chunks at runtime).

See the [Morton ND LUT Usage Guide](docs/MortonND_LUT.md) for details.

```c++
// Generates a 4D LUT encoder (4 fields, 16 bits each, 8-bit LUT) using the compiler.
constexpr auto MortonND_4D = mortonnd::MortonNDLutEncoder<4, 16, 8>();

// Encodes 4 fields. Can be done at run-time or compile-time.
auto encoding = MortonND_4D.Encode(f1, f2, f3, f4);
```

## Performance
Performance metrics were gathered using [this fork](https://github.com/kevinhartman/libmorton#fork-changes) of @Forceflow's [libmorton](https://github.com/Forceflow/libmorton) library. Libmorton contains a suite of different Morton encode/decode algorithms for 2D and 3D. The snippets below show performance comparisons to the 3D algorithms found there, as well as comparisons between different 3D configurations of Morton ND.

To run these tests on your own machine, clone the fork linked above.

The following metrics (sorted by random access time, ascending) were collected on an i7-6920HQ, compiled with GCC 8.1 on macOS 10.13 using "-O3 -DNDEBUG". Results include data from both linearly increasing and random inputs to demonstrate the performance impact of cache (hit or miss) under each algorithm / configuration. Results are averaged over 5 runs (each algorithm is run 5 times consecutively before moving on to the next).

### 32-bit
```
++ Running each performance test 5 times and averaging results
++ Encoding 512^3 morton codes (134217728 in total)

    Linear      Random
    ======      ======
    643.219 ms  610.135 ms  : 32-bit (MortonND)    LUT: 1 chunks, 10 bit LUT  (fastest, random)
    635.962 ms  624.285 ms  : 32-bit (MortonND)    BMI2
    634.450 ms  626.686 ms  : 32-bit (lib-morton)  BMI2 instruction set       (fastest, linear)
    724.331 ms  718.027 ms  : 32-bit (lib-morton)  LUT Shifted
    736.239 ms  724.107 ms  : 32-bit (MortonND)    LUT: 2 chunks, 8 bit LUT
    748.223 ms  733.092 ms  : 32-bit (lib-morton)  LUT
    793.999 ms  786.195 ms  : 32-bit (MortonND)    LUT: 2 chunks, 5 bit LUT
    1047.713 ms 1028.900 ms : 32-bit (lib-morton)  Magicbits
    1656.251 ms 1649.317 ms : 32-bit (lib-morton)  For ET
    1800.730 ms 1792.513 ms : 32-bit (lib-morton)  For
```

### 64-bit
```
++ Running each performance test 5 times and averaging results
++ Encoding 512^3 morton codes (134217728 in total)

    Linear      Random
    ======      ======
    674.413 ms  661.722 ms  : 64-bit (lib-morton)  BMI2 instruction set       (fastest)
    677.112 ms  674.089 ms  : 64-bit (MortonND)    BMI2
    959.701 ms  951.082 ms  : 64-bit (MortonND)    LUT: 3 chunks, 7 bit LUT
    747.045 ms  967.286 ms  : 64-bit (MortonND)    LUT: 2 chunks, 16 bit LUT
    973.362 ms  977.558 ms  : 64-bit (lib-morton)  LUT Shifted
    1043.979 ms 1041.137 ms : 64-bit (lib-morton)  LUT
    634.308 ms  1095.827 ms : 64-bit (MortonND)    LUT: 1 chunks, 21 bit LUT
    1261.956 ms 1249.309 ms : 64-bit (lib-morton)  Magicbits
    3619.233 ms 3629.182 ms : 64-bit (lib-morton)  For
    2310.747 ms 3909.334 ms : 64-bit (lib-morton)  For ET
```

## Thanks
* Jeroen Baert (@Forceflow)
  - [Morton encoding/decoding through bit interleaving: Implementations](https://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/)
  - [libmorton](https://github.com/Forceflow/libmorton), a C++ header-only library for 2D and 3D Morton encoding and decoding, which includes non-LUT-based implementations (Magicbits, BMI2, etc.).

## License
This project is licensed under the MIT license.

Attribution is appreciated where applicable, and as such, a NOTICE file is included which may be distributed in the credits of derivative software.
