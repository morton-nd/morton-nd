# Morton ND BMI2 Usage Guide
The `MortonNDBmi` class can be used to encode and decode `32` and `64` bit width fields in N dimensions using the BMI2 instruction set, available on modern x64 CPUs (Haswell (Intel), Excavator (AMD) and newer).

Configure the class by providing the number of fields `Fields` followed by the result type `T` (either `uint32_t` or `uint64_t`) as template parameters.

The number of bits used (starting with the LSb) of each input field provided to the `Encode` function (and conversely, returned from the `Decode` function) is calculated as `⌊bits in T / Fields⌋`. Any higher-order bits will be ignored during an encode and will be `0` during a decode. For this reason, it is not necessary to mask off high-order bits.

For diagnostic purposes, the member `FieldBits` provides the number of bits considered in each field. For example, `MortonNDBmi<3, uint64_t>::FieldBits` will return `21`.

### Encoding
The encode function is variadic, but will assert that exactly `Fields` fields are specified. 

```c++
// 32-bit 3D, 10 bits per field (MortonNDBmi<3, uint32_t>::FieldBits == 10)
using MortonND_3D_32 = mortonnd::MortonNDBmi<3, uint32_t>;

const uint32_t field1 = 9;    // 9.
const uint32_t field2 = 5;    // 5.
const uint32_t field3 = 1025; // 1, but with a dirty upper bit (the 11th LSb is set).

// The encoding of 9, 5, and 1
// 
// field3 will be intepreted as 1. The 11th bit is set (1024), but is automatically ignored.
auto encoding = MortonND_3D_32.Encode(field1, field2, field3);
```

### Decoding
The decode function returns a `tuple` of each field decoded from the encoding.

```c++
uint32_t d_field1;
uint32_t d_field2;
uint32_t d_field3;

// Assign the decoded values (9, 5, and 1) to respective fields.
std::tie(d_field1, d_field2, d_field3) = MortonND_3D_32.Decode(encoding);
```

## Compiling
* The `MortonNDBmi` class is conditionally compiled based on the definition of `__BMI2__` (or `__AVX2__` for MSVC), which GCC and Clang will define automatically if invoked with `-mbmi2`. If using MSVC, set your project to use Enhanced Instruction Set "Advanced Vector Extensions 2 (/arch:AVX2)".
