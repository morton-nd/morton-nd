# Morton ND BMI2 Usage Guide
The `MortonNDBmi` class can be used to encode and decode `32` and `64` bit width fields in N dimensions using the BMI2 instruction set, available on modern x64 CPUs (Haswell (Intel), Excavator (AMD) and newer).

Configure the class by providing the number of fields `Dimensions` followed by the result type `T` (either `uint32_t` or `uint64_t`) as template parameters.

The number of bits used (starting with the LSb) of each input field provided to the `Encode` function (and conversely, returned from the `Decode` function) is calculated as `⌊bits in T / Fields⌋`. Any higher-order bits will be ignored during an encode and will be `0` during a decode. For this reason, **it is not necessary to mask off high-order bits.**

For diagnostic purposes, the member `FieldBits` provides the number of bits considered in each field. For example, `MortonNDBmi<3, uint64_t>::FieldBits` will return `21`.

### Encoding
The encode function is variadic, but will assert that exactly `Dimensions` fields are specified. 

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

## Code Generation
This section shows the x86_64 machine code generated for both encoding and decoding in 4D. The snippets were compiled using Clang 6.0.0 with options: `-mbmi2 -O3`.

### 4D Encoding
#### Program
```c++
#include <morton-nd/mortonND_BMI2.h>

uint16_t x = 1, y = 2, z = 3, w = 4;
int main() {
    return mortonnd::MortonNDBmi<4, uint32_t>::Encode(x, y, z, w);
}
```

#### Generated code
```asm
main:
        movzx   eax, word ptr [rip + x]   # Load x, y, z, and w into registers
        movzx   ecx, word ptr [rip + y]
        movzx   edx, word ptr [rip + z]
        movzx   esi, word ptr [rip + w]
        mov     edi, -2004318072          # Load w mask (1000 1000 1000 1000 1000 1000 1000 1000) into edi
        pdep    esi, esi, edi             # Deposit w, spreading over mask (edi)
        mov     edi, 1145324612           # Load z mask (0100 0100 0100 0100 0100 0100 0100 0100)
        pdep    edx, edx, edi             # Deposit
        or      edx, esi                  # OR deposited w and z
        mov     esi, 572662306            # Load y mask (0010 0010 0010 0010 0010 0010 0010 0010)
        pdep    ecx, ecx, esi             # Deposit
        mov     esi, 286331153            # Load x mask (0001 0001 0001 0001 0001 0001 0001 0001)
        pdep    eax, eax, esi             # Deposit
        or      eax, ecx                  # OR deposited x and y
        or      eax, edx                  # OR results of ORs above
        ret
```

### 4D Decoding
#### Program
```c++
#include <morton-nd/mortonND_BMI2.h>

uint32_t encoding = 1095;
uint16_t x, y, z, w;
int main() {
    std::tie(x, y, z, w) = mortonnd::MortonNDBmi<4, uint32_t>::Decode(encoding);
}
```

#### Generated code
```asm
main:
        mov     eax, dword ptr [rip + encoding]   # Load encoding into register
        mov     ecx, 286331153                    # Load x mask
        pext    ecx, eax, ecx                     # Extract x from encoding using mask
        mov     edx, 572662306                    # Load y mask
        pext    edx, eax, edx                     # Extract y from encoding
        mov     esi, 1145324612                   # Load z mask
        pext    esi, eax, esi                     # Extract z from encoding
        mov     edi, -2004318072                  # Load w mask
        mov     word ptr [rip + x], cx            # Store lower 16 bits of x
        mov     word ptr [rip + y], dx            # Store lower 16 bits of y
        mov     word ptr [rip + z], si            # Store lower 16 bits of z
        pext    eax, eax, edi                     # Extract w from encoding
        mov     word ptr [rip + w], ax            # Store lower 16 bits of w
        xor     eax, eax                          # Clear eax
        ret
```
