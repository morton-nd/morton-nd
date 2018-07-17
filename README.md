# Morton ND
A C++14 header-only Morton encoding library for N dimensions, based on the Lookup Table (LUT) method. LUTs are defined with constant expressions and thus can be generated (and even used) at compile-time.

The encoder supports chunking, meaning input fields can be automatically split into parts prior to lookup. The results of these partial lookups are then combined. This allows a smaller LUT to be used at the expense of the extra 'shift' and 'or' operations required to combine the results.

## Usage
The MortonNDEncoder class encapsulates a LUT and provides a corresponding encode function. To instantiate it, you must specify the number of fields (N), the number of chunks in each field, and the size of each chunk in bits as template parameters.

Field size in bits is calculated as the number of chunks multiplied by the size of each chunk in bits (chunks * chunk size).

### Example: 2D encoding
For a 2 field encoder, where the result must fit in a 32-bit field, the max field size is 16 bits (32 bits / 2 fields).

Therefore, to create a suitable MortonNDEncoder, you must parameterize it such that chunks * bits per chunk = 16:

```c++
// 1) 1 chunk, 16 bits. LUT size is 2^16 entries * 4 bytes per entry = 262.144 KB
constexpr auto MortonND_2D_32 = mortonnd::MortonNDEncoder<2, 1, 16>();

// 2) 2 chunks, 8 bits. LUT size is 2^8 entries * 2 bytes per entry = 512 bytes
constexpr auto MortonND_2D_32 = mortonnd::MortonNDEncoder<2, 2, 8>();

// 3) 4 chunks, 4 bits. LUT size is 2^4 entires * 1 byte per entry = 16 bytes
constexpr auto MortonND_2D_32 = mortonnd::MortonNDEncoder<2, 4, 4>();

// 4) 8 chunks, 2 bits
// ... this is supported (as is 16 chunks, 1 bit), but probably not very useful
```

Note the size of each LUT. A 16-bit chunk size requires a few hundred KB, but will provide the fastest encoding. A 4-bit chunk size only requires a 16 byte LUT, but each field will require roughly 3 times the number of lookup, shifting, or-ing and masking operations. See the performance section below for a comparison.

The encode function is variadic, but will assert that exactly 2 fields are specified for 2D (and N for ND):
```c++
auto encoding = MortonND_2D_32.Encode(9, 5);
```

If targeting a 64-bit result, the max field size is 32 bits (64 bits / 2 fields). However, in this case, the max chunk size is still 16 (not 32). This is due to compiler limitations which prevent a LUT of size 2^32 from being generated (which would be greater than 17 GB in size anyway – likely impractical). See the notes section below on various compiler limitations.
