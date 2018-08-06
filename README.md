# Morton ND
A C++14 header-only Morton encoding library for N dimensions, based on the Lookup Table (LUT) method. LUTs are defined with constant expressions and thus can be generated (and even used) at compile-time.

The encoder supports chunking, allowing a LUT smaller than the input field width to be used internally. This is useful for applications which require faster compilation times and smaller binaries (at the expense of extra bit manipulation operations required to combine chunks at runtime).

```c++
// Generates a 4D LUT encoder (4 fields, 16 bits each, 8-bit LUT) using the compiler.
constexpr auto MortonND_4D = mortonnd::MortonNDEncoder<4, 16, 8>();

// Encodes 4 fields. Can be done at run-time or compile-time.
auto encoding = MortonND_4D.Encode(f1, f2, f3, f4);
```

## Performance
Performance metrics were gathered using [this fork](https://github.com/kevinhartman/libmorton) of @Forceflow's [libmorton](https://github.com/Forceflow/libmorton) library. Libmorton contains a suite of different Morton encode/decode algorithms for 2D and 3D. The snippets below show performance comparisons to the 3D algorithms found there, as well as comparisons between different 3D Morton ND LUT size configurations. Results are averaged over 5 runs (each algorithm is run 5 times consecutively before moving on to the next).

libmorton also includes an approach using the BMI2 instruction set, the performance of which is not captured here (due to incompatible test environment).

To run these tests on your own machine, clone the fork linked above.

The following metrics (sorted by random access time, ascending) were collected on an i7-6920HQ, compiled with GCC 8.1 on macOS 10.13 using "-O3 -DNDEBUG". Results include data from both linearly increasing and random inputs to demonstrate the performance impact of cache (hit or miss) under each algorithm / configuration.

### 32-bit
```
++ Running each performance test 5 times and averaging results
++ Encoding 512^3 morton codes (134217728 in total)

    Linear      Random
    ======      ======
    651.950 ms  633.010 ms  : 32-bit (MortonND)    1 chunks, 10 bit LUT  (fastest)
    759.446 ms  740.276 ms  : 32-bit (MortonND)    2 chunks, 8 bit LUT
    749.832 ms  745.619 ms  : 32-bit (lib-morton)  LUT Shifted
    790.699 ms  767.882 ms  : 32-bit (lib-morton)  LUT
    818.037 ms  806.331 ms  : 32-bit (MortonND)    2 chunks, 5 bit LUT
    1118.599 ms 1066.320 ms : 32-bit (lib-morton)  Magicbits
    1712.808 ms 1722.190 ms : 32-bit (lib-morton)  For ET
    2084.808 ms 1908.658 ms : 32-bit (lib-morton)  For
```

### 64-bit
```
++ Running each performance test 5 times and averaging results
++ Encoding 512^3 morton codes (134217728 in total)

    Linear      Random
    ======      ======
    1004.443 ms 988.571 ms  : 64-bit (MortonND)    3 chunks, 7 bit LUT   (fastest, random)
    994.033 ms  1000.821 ms : 64-bit (lib-morton)  LUT Shifted
    792.387 ms  1021.745 ms : 64-bit (MortonND)    2 chunks, 16 bit LUT
    1120.427 ms 1121.624 ms : 64-bit (lib-morton)  LUT
    672.983 ms  1207.028 ms : 64-bit (MortonND)    1 chunks, 21 bit LUT  (fastest, linear)
    1285.695 ms 1305.508 ms : 64-bit (lib-morton)  Magicbits
    6078.532 ms 6066.963 ms : 64-bit (lib-morton)  For
    3568.369 ms 6542.374 ms : 64-bit (lib-morton)  For ET
```

## Usage
The `MortonNDEncoder` class encapsulates a LUT and provides a corresponding encode function. To instantiate it, you must specify the number of fields (N), the number of bits in each field (starting with the least significant bit), and the size of the LUT as template parameters. LUT size is expressed as the number of bits which will be looked up at a time (chunk size). For example, a LUT size of 16 yields a LUT with 2^16 entries, allowing 16 bit lookups.

To create a `MortonNDEncoder` to accommodate N fields, each Q bits, parameterize it with `N` for `Fields`,  and `Q` for `FieldBits`. Use the third template parameter, `LutBits`, to specify the width of each lookup chunk. 

At both extremes, a  `LutBits` value equal to `FieldBits` will result in a single lookup (1 chunk), whereas a value of `1` will result in `|FieldBits|` lookups (`|FieldBits|` chunks). For every additional chunk, additional bit manipulation operations will be required to combine results (increasing encode time), however, the `MortonNDEncoder` class was written with compiler optimization in mind and should not introduce additional function calls. The number of chunks which will be used based on your configuration is exposed as a static member field to aid in performance debugging.

While large LUTs offer the least bit manipulation overhead, they have a few drawbacks: compilation time increases significantly with larger values, they take up more space in the program image / executable, and they can be slower than smaller LUTs due to cache misses, depending on the domain's access pattern.

<blockquote>
<b>Note:</b></p>
The max LUT size is 24 when compiled with GCC (8.1) and 25 for Clang (900.0.39.2). See the notes section below on various compiler limitations.
</blockquote>

### Encoding
Once you've instantiated a `MortonNDEncoder`, use its `Encode` function to encode inputs. The encode function is variadic, but will assert that exactly N fields are specified. Note that this function is also marked `constexpr` and can be used in compile-time expressions.

The upper unused bits of the input fields provided to the Encode function must first be cleared. For example, if the encoder is parameterized with a `FieldBits` value of `10`, then any bits beyond the 10th LSb must be 0. This can be done easily by `and`ing the field with `MortonNDEncoder<...>::InputMask`.

```c++
// encoding in 32-bit 3D, 10 bits per field
constexpr auto MortonND_3D_32 = mortonnd::MortonNDEncoder<3, 10, 10>();

const uint32_t field1 = 9;    // 9.
const uint32_t field2 = 5;    // 5.
const uint32_t field3 = 1025; // 1, but with a dirty upper bit (the 11th LSb is set).

// The encoding of 9, 5, and 1
// 
// Note that the first two fields (9 and 5) don't require masking, but field3 (1) does since an upper unused bit is set.
auto encoding = MortonND_3D_32.Encode(field1, field2, field3 & MortonND_3D_32::InputMask);
```

### Example: 2D encoding (N = 2)
For a 2 field encoder, where the result must fit in a 32-bit field, the max field size is 16 bits (32 bits / 2 fields).

Examples configurations of suitable `MortonNDEncoder`s for 2 fields (16 bits each):

```c++
// 1) 1 chunk, 16 bits. LUT size is 2^16 entries * 4 bytes per entry = 262.144 KB
constexpr auto MortonND_2D_32 = mortonnd::MortonNDEncoder<2, 16, 16>();

// 2) 2 chunks, 8 bits. LUT size is 2^8 entries * 2 bytes per entry = 512 bytes
constexpr auto MortonND_2D_32 = mortonnd::MortonNDEncoder<2, 16, 8>();

// 3) 4 chunks, 4 bits. LUT size is 2^4 entires * 1 byte per entry = 16 bytes
constexpr auto MortonND_2D_32 = mortonnd::MortonNDEncoder<2, 16, 4>();
```

Note the size of each LUT. A 16-bit chunk size requires a few hundred KB, but will likely provide the fastest encoding. A 4-bit chunk size only requires a 16 byte LUT, but each field will require roughly 3 times the number of lookup, shifting, or-ing and masking operations.

### Example: 3D encoding (N = 3)
For a 3 field encoder, where the result must fit in a 64 bit field, the max size of each field is 21 bits (⌊64 bits / 3 fields⌋).

`MortonNDEncoder` for 3 fields, each 21 bits:

```c++
// 1) 1 chunk, 21 bits. LUT size is 2^21 entries * 8 bytes per entry = 16.777216 MB
constexpr auto MortonND_3D_64 = mortonnd::MortonNDEncoder<3, 21, 21>();

// 2) 2 chunk, 16 bits. LUT size is 2^16 entries * 8 bytes per entry = 524.288 KB
constexpr auto MortonND_3D_64 = mortonnd::MortonNDEncoder<3, 21, 16>();

// 3) 3 chunks, 7 bits. LUT size is 2^7 entries * 4 bytes per entry = 512 bytes
constexpr auto MortonND_3D_64 = mortonnd::MortonNDEncoder<3, 21, 7>();

auto encoding = MortonND_3D_64.Encode(9, 5, 1);
```

For a 3 field encoder, where the result must fit in a 32 bit field, max field size = ⌊32 bits / 3 fields⌋ = 10 bits.

`MortonNDEncoder` for 3 fields, each 10 bits:

```c++
// 1 chunk, 10 bits.
constexpr auto MortonND_3D_32 = mortonnd::MortonNDEncoder<3, 10, 10>();

// 2 chunks, 5 bits.
constexpr auto MortonND_3D_32 = mortonnd::MortonNDEncoder<3, 10, 5>();
```

### Example: 5D encoding (N = 5)
For a 5 field encoder, where the result must fit in a 64 bit field, the max size of each field is 12 bits (⌊64 bits / 5 fields⌋).

`MortonNDEncoder` for 5 fields, each 12 bits:

```c++
// 1) 1 chunk, 12 bits. LUT size is 2^12 entries * 8 bytes per entry = 32.768 KB
constexpr auto MortonND_5D_64 = mortonnd::MortonNDEncoder<5, 12, 12>();

// 2) 2 chunks, 6 bits. LUT size is 2^6 entries * 4 bytes per entry = 256 bytes
constexpr auto MortonND_3D_64 = mortonnd::MortonNDEncoder<5, 12, 6>();

// 3) 3 chunks, 4 bits. LUT size is 2^4 entries * 4 bytes per entry = 64 bytes
constexpr auto MortonND_3D_64 = mortonnd::MortonNDEncoder<5, 12, 4>();

auto encoding = MortonND_3D_64.Encode(17, 13, 9, 5, 1);
```

## Compiling
* Expect long compilation times with a large LUT size (`LutBits`). The max LUT size is 24 when compiled with GCC (8.1) and 25 for Clang (900.0.39.2).
* Compile with release/optimization flags for accurate performance.
* VC++ is untested.

## Thanks
* Jeroen Baert (@Forceflow)
  - [Morton encoding/decoding through bit interleaving: Implementations](https://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/)
  - [libmorton](https://github.com/Forceflow/libmorton), a C++ header-only library for 2D and 3D Morton encoding *and* decoding, which includes non-LUT-based implementations (Magicbits, BMI2, etc.).

## License
This project is licensed under the MIT license.

Attribution is appreciated where applicable, and as such, a NOTICE file is included which may be distributed in the credits of derivative software.
