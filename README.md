# Morton ND
A C++14 header-only Morton encoding library for N dimensions, based on the Lookup Table (LUT) method. LUTs are defined with constant expressions and thus can be generated (and even used) at compile-time.

The encoder supports chunking, meaning input fields can be automatically split into parts prior to lookup. The results of these partial lookups are then combined. This allows a smaller LUT to be used at the expense of the extra 'shift' and 'or' operations required to combine the results.

## Usage
The MortonNDEncoder class encapsulates a LUT and provides a corresponding encode function. To instantiate it, you must specify the number of fields (N), the number of bits in each field (starting with the least significant bit), and the size of the LUT as template parameters. LUT size is expressed as the number of bits which will be looked up at a time (chunk size). For example, a LUT size of 16 yields a LUT with 2^16 entries, allowing 16 bit lookups.

To create a MortonNDEncoder to accommodate N fields, each Q bits, parameterize it with `N` for `Fields`,  and `Q` for `FieldBits`. Use the third template parameter, `LutBits`, to specify the width of each lookup chunk. 

At both extremes, a  `LutBits` value equal to `FieldBits` will result in a single lookup (1 chunk), whereas a value of `1` will result in #`FieldBits` lookups (#`FieldBits` chunks). For every additional chunk, additional bit manipulation operations will be required to combine results (increasing encode time), however, the MortonNDEncoder was written with compiler optimization in mind and should not introduce additional function calls. The number of chunks which will be used based on your configuration is exposed as a static member field.

While large LUTs offer the least bit manipulation overhead, they have a few drawbacks: compilation time increases significantly with larger values, they take up more space in the program image / executable, and they can be slower than smaller LUTs due to cache misses, depending on the domain's access pattern.

<blockquote>
<b>Note:</b></p>
The max LUT size using GCC is 27 and 16 for Clang (at the time of writing). See the notes section below on various compiler limitations.
</blockquote>

### Example: 2D encoding (N = 2)
For a 2 field encoder, where the result must fit in a 32-bit field, the max field size is 16 bits (32 bits / 2 fields).

Examples of suitable MortonNDEncoder for 2 fields, each 16 bits:

```c++
// 1) 1 chunk, 16 bits. LUT size is 2^16 entries * 4 bytes per entry = 262.144 KB
constexpr auto MortonND_2D_32 = mortonnd::MortonNDEncoder<2, 16, 16>();

// 2) 2 chunks, 8 bits. LUT size is 2^8 entries * 2 bytes per entry = 512 bytes
constexpr auto MortonND_2D_32 = mortonnd::MortonNDEncoder<2, 16, 8>();

// 3) 4 chunks, 4 bits. LUT size is 2^4 entires * 1 byte per entry = 16 bytes
constexpr auto MortonND_2D_32 = mortonnd::MortonNDEncoder<2, 16, 4>();
```

Note the size of each LUT. A 16-bit chunk size requires a few hundred KB, but will likely provide the fastest encoding. A 4-bit chunk size only requires a 16 byte LUT, but each field will require roughly 3 times the number of lookup, shifting, or-ing and masking operations.

The encode function is variadic, but will assert that exactly 2 fields are specified for 2D (and N for ND):
```c++
auto encoding = MortonND_2D_32.Encode(9, 5);
```

### Example: 3D encoding (N = 3)
For a 3 field encoder, where the result must fit in a 64 bit field, the max size of each field is 21 bits (⌊64 bits / 3 fields⌋).

Examples of suitable MortonNDEncoder for 3 fields, each 21 bits:

```c++
// 1) 1 chunk, 21 bits. LUT size is 2^21 entries * 8 bytes per entry = 16.777216 MB
constexpr auto MortonND_3D_64 = mortonnd::MortonNDEncoder<3, 21, 21>();

// 2) 2 chunk, 16 bits. LUT size is 2^16 entries * 8 bytes per entry = 524.288 KB
constexpr auto MortonND_3D_64 = mortonnd::MortonNDEncoder<3, 21, 16>();

// 3) 3 chunks, 7 bits. LUT size is 2^7 entries * 4 bytes per entry = 512 bytes
constexpr auto MortonND_3D_64 = mortonnd::MortonNDEncoder<3, 21, 7>();

auto encoding = MortonND_3D_64.Encode(9, 5, 1);
```

Examples of a 3 field encoder, where the result must fit in a 32 bit field (max field size = ⌊32 bits / 3 fields⌋ = 10 bits):

```c++
// 1 chunk, 10 bits.
constexpr auto MortonND_3D_32 = mortonnd::MortonNDEncoder<3, 10, 10>();

// 2 chunks, 5 bits.
constexpr auto MortonND_3D_32 = mortonnd::MortonNDEncoder<3, 10, 5>();
```

### Example: 5D encoding (N = 5)
For a 5 field encoder, where the result must fit in a 64 bit field, the max size of each field is 12 bits (⌊64 bits / 5 fields⌋).

Example MortonNDEncoders:

```c++
// 1) 1 chunk, 12 bits. LUT size is 2^12 entries * 8 bytes per entry = 32.768 KB
constexpr auto MortonND_5D_64 = mortonnd::MortonNDEncoder<5, 12, 12>();

// 2) 2 chunks, 6 bits. LUT size is 2^6 entries * 4 bytes per entry = 256 bytes
constexpr auto MortonND_3D_64 = mortonnd::MortonNDEncoder<5, 12, 6>();

// 3) 3 chunks, 4 bits. LUT size is 2^4 entries * 4 bytes per entry = 64 bytes
constexpr auto MortonND_3D_64 = mortonnd::MortonNDEncoder<5, 12, 4>();

auto encoding = MortonND_3D_64.Encode(17, 13, 9, 5, 1);
```

## 3D Performance
Performance metrics were gathered using [Forceflow's libmorton library](https://github.com/Forceflow/libmorton), which contains a suite of different Morton encode/decode algorithms for 2D and 3D. The snippets below show performance comparisons to the algorithms found there, as well as comparisons between different Morton ND LUT size configurations.

libmorton also includes an approach using the BMI2 instruction set, the performance of which is not captured here (due to incompatible test hardware), as well as a suite of Morton decoders for 2D and 3D applications.

Depending on your use case and hardware, a particular configuration of Morton ND or perhaps one of the many libmorton algorithms may offer the best encoding performance.

The following metrics were collected on an i7-6920HQ (6th generation Quad-core Intel 2.9GHz mobile CPU), compiled with GCC 8.1 on macOS 10.13 using "-O3 -DNDEBUG".

### 32-bit
```
++ Encoding 512^3 morton codes (134217728 in total)

    Linear      Random
    ======      ======
    925.439 ms  874.468 ms  : 32-bit MortonND: 2 chunks, 5 bits
    743.166 ms  711.925 ms  : 32-bit MortonND: 1 chunk, 10 bits  (fastest)
    
    libmorton algorithms
    ====================
    2663.162 ms 2599.181 ms : 32-bit For
    2330.065 ms 2014.106 ms : 32-bit For ET
    1171.010 ms 1135.522 ms : 32-bit Magicbits
    857.056 ms  814.186 ms  : 32-bit LUT
    831.843 ms  816.522 ms  : 32-bit LUT Shifted
```

### 64-bit
```
++ Encoding 512^3 morton codes (134217728 in total)

    Linear      Random
    ======      ======
    1349.667 ms 1329.539 ms : 64-bit MortonND: 3 chunks, 7 bits  (fastest, random)
    894.632 ms  1657.213 ms : 64-bit MortonND: 1 chunk, 21 bits  (fastest, linear)
    
    libmorton algorithms
    ====================
    1354.036 ms 1357.236 ms : 64-bit LUT Shifted
    1471.270 ms 1467.422 ms : 64-bit LUT
    1759.593 ms 1756.538 ms : 64-bit Magicbits
    4726.462 ms 8649.010 ms : 64-bit For ET
    8279.347 ms 8258.069 ms : 64-bit For

```

## Compiling
* GCC is much faster than Clang when generating LUTs at compile-time. Expect long compilation times with a large chunk size (LUT size) when using Clang. GCC can also handle up to a chunk size of 27 bits (28+ fail gracefully), whereas Clang will crash with a chunk size > 16 bits. For this reason, use GCC for large LUTs if possible.
* Compile with release/optimization flags.
* VC++ is untested.
