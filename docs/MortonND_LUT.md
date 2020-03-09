# Morton ND LUT Usage Guide
The `MortonNDLutEncoder` class encapsulates a LUT and provides a corresponding encode function. To instantiate it, you must specify the number of fields (N), the number of bits in each field (starting with the least significant bit), and the size of the LUT as template parameters. LUT size is expressed as the number of bits which will be looked up at a time (chunk size). For example, a LUT size of 16 yields a LUT with 2^16 entries, allowing 16 bit lookups.

To create a `MortonNDLutEncoder` to accommodate `N` fields, each `Q` bits, parameterize it with `Dimensions = N`  and `FieldBits = Q`. Use the third template parameter, `LutBits`, to specify the width of each lookup chunk.

At both extremes, a  `LutBits` value equal to `FieldBits` will result in a single lookup (1 chunk), whereas a value of `1` will result in `|FieldBits|` lookups (`|FieldBits|` chunks). For every additional chunk, additional bit manipulation operations will be required to combine results (increasing encode time), however, the `MortonNDLutEncoder` class was written with compiler optimization in mind and should not introduce additional function calls. The number of chunks which will be used based on your configuration is exposed as a static member field (`MortonNDLutEncoder<...>::ChunkCount`) to aid in performance debugging.

While large LUTs offer the least bit manipulation overhead, they have a few drawbacks: compilation time increases significantly with larger values, they take up more space in the program image / executable, and they can be slower than smaller LUTs due to cache misses, depending on the domain's access pattern.

>
> **Note:**
>
> The max LUT size is 24 when compiled with GCC (8.1), 25 for Clang (900.0.39.2), and 10 for MSVC (1915). See the notes section below on various compiler limitations.

### Encoding
Once you've instantiated a `MortonNDLutEncoder`, use its `Encode` function to encode inputs. The encode function is variadic, but will assert that exactly N fields are specified. Note that this function is also marked `constexpr` and can be used in compile-time expressions.

The upper unused bits of the input fields provided to the Encode function must first be cleared. For example, if the encoder is parameterized with a `FieldBits` value of `10`, then any bits beyond the 10th LSb must be 0. When `T` is a native integer type, this can be done easily by logically AND-ing the field with `MortonNDLutEncoder<...>::InputMask()`.

> Note: `InputMask()` is not available when `T` is a user-defined class.

```c++
// encoding in 32-bit 3D, 10 bits per field
constexpr auto MortonND_3D_32 = mortonnd::MortonNDLutEncoder<3, 10, 10>();

const uint32_t field1 = 9;    // 9.
const uint32_t field2 = 5;    // 5.
const uint32_t field3 = 1025; // 1, but with a dirty upper bit (the 11th LSb is set).

// The encoding of 9, 5, and 1
// 
// Note that the first two fields (9 and 5) don't require masking, but field3 (1) does since an upper unused bit is set.
auto encoding = MortonND_3D_32.Encode(field1, field2, field3 & MortonND_3D_32::InputMask());
```

### Example: 2D encoding (N = 2)
For a 2 field encoder, where the result must fit in a 32-bit field, the max field size is 16 bits (32 bits / 2 fields).

Example configurations of suitable `MortonNDLutEncoder`s for 2 fields (16 bits each):

```c++
// 1) 1 chunk, 16 bits. LUT size is 2^16 entries * 4 bytes per entry = 262.144 KB
constexpr auto MortonND_2D_32 = mortonnd::MortonNDLutEncoder<2, 16, 16>();

// 2) 2 chunks, 8 bits. LUT size is 2^8 entries * 2 bytes per entry = 512 bytes
constexpr auto MortonND_2D_32 = mortonnd::MortonNDLutEncoder<2, 16, 8>();

// 3) 4 chunks, 4 bits. LUT size is 2^4 entires * 1 byte per entry = 16 bytes
constexpr auto MortonND_2D_32 = mortonnd::MortonNDLutEncoder<2, 16, 4>();
```

Note the size of each LUT. A 16-bit chunk size requires a few hundred KB, but will likely provide the fastest encoding. A 4-bit chunk size only requires a 16 byte LUT, but each field will require roughly 3 times the number of lookup, shifting, or-ing and masking operations.

### Example: 3D encoding (N = 3)
For a 3 field encoder, where the result must fit in a 64 bit field, the max size of each field is 21 bits (⌊64 bits / 3 fields⌋).

`MortonNDLutEncoder` for 3 fields, each 21 bits:

```c++
// 1) 1 chunk, 21 bits. LUT size is 2^21 entries * 8 bytes per entry = 16.777216 MB
constexpr auto MortonND_3D_64 = mortonnd::MortonNDLutEncoder<3, 21, 21>();

// 2) 2 chunk, 16 bits. LUT size is 2^16 entries * 8 bytes per entry = 524.288 KB
constexpr auto MortonND_3D_64 = mortonnd::MortonNDLutEncoder<3, 21, 16>();

// 3) 3 chunks, 7 bits. LUT size is 2^7 entries * 4 bytes per entry = 512 bytes
constexpr auto MortonND_3D_64 = mortonnd::MortonNDLutEncoder<3, 21, 7>();

auto encoding = MortonND_3D_64.Encode(9, 5, 1);
```

For a 3 field encoder, where the result must fit in a 32 bit field, max field size = ⌊32 bits / 3 fields⌋ = 10 bits.

`MortonNDLutEncoder` for 3 fields, each 10 bits:

```c++
// 1 chunk, 10 bits.
constexpr auto MortonND_3D_32 = mortonnd::MortonNDLutEncoder<3, 10, 10>();

// 2 chunks, 5 bits.
constexpr auto MortonND_3D_32 = mortonnd::MortonNDLutEncoder<3, 10, 5>();
```

### Example: 5D encoding (N = 5)
For a 5 field encoder, where the result must fit in a 64 bit field, the max size of each field is 12 bits (⌊64 bits / 5 fields⌋).

`MortonNDLutEncoder` for 5 fields, each 12 bits:

```c++
// 1) 1 chunk, 12 bits. LUT size is 2^12 entries * 8 bytes per entry = 32.768 KB
constexpr auto MortonND_5D_64 = mortonnd::MortonNDLutEncoder<5, 12, 12>();

// 2) 2 chunks, 6 bits. LUT size is 2^6 entries * 4 bytes per entry = 256 bytes
constexpr auto MortonND_3D_64 = mortonnd::MortonNDLutEncoder<5, 12, 6>();

// 3) 3 chunks, 4 bits. LUT size is 2^4 entries * 4 bytes per entry = 64 bytes
constexpr auto MortonND_3D_64 = mortonnd::MortonNDLutEncoder<5, 12, 4>();

auto encoding = MortonND_3D_64.Encode(17, 13, 9, 5, 1);
```

## Compiling
* Expect long compilation times with a large LUT size (`LutBits`). The max LUT size is 24 when compiled with GCC (8.1) and 25 for Clang (900.0.39.2).
* Compile with release/optimization flags for accurate performance.
* MSVC supports a maximum LUT size of 10. When configuring a LUT for use with MSVC, always choose to minimize the number of chunks needed over a smaller LUT (i.e. cache locality). This is recommended because MSVC unfortunately will not unroll the loops required to combine chunks, and this performance hit seems to out-weigh any cache benefit of a smaller LUT. For example, to encode 16-bit fields (`FieldsBits = 16`), at least 2 chunks are required since `⌊16 / (10, the max LUT size)⌋ = 2`. You can minimize the LUT size to `8` while maintaining a chunk count of 2, but any smaller will increase chunk count, worsening performance.
