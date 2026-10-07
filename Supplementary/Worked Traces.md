# Worked Traces

Longer hand traces live here. Organize by mechanism so later chunks can extend the same trace with deeper representation, math, implementation, or hardware behavior.

A trace should include a concrete input, every state transition that matters, the final output, one invariant check, and one deliberately failing or boundary input when useful.

## M001 - The 0xAD control byte

**Introduced by:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 / M001]]

### Input and conventions

Schema, from least to most significant position:

```text
bits 0-2 mode | bit 3 visible | bits 4-5 quality | bit 6 locked | bit 7 dirty
```

Input: `{mode=5, visible=1, quality=2, locked=0, dirty=1}`. Field zero occupies the least-significant available bits.

### State-by-state trace

```text
mode       5 << 0 = 00000101
visible    1 << 3 = 00001000
quality    2 << 4 = 00100000
locked     0 << 6 = 00000000
dirty      1 << 7 = 10000000
OR result           10101101 = 0xAD
```

Base interpretations:

```text
binary 10101101 = 128 + 32 + 8 + 4 + 1 = decimal 173
hex    1010 1101 = A D                    = 0xAD
octal  010 101 101 = 2 5 5                = 0255
```

Quality extraction:

```text
10101101 >> 4 = 00001010
00001010 & 03 = 00000010 = 2
```

LSB-first bit-array packing of `[1,0,1,1,0,1,0,1]` accumulates `01`, `05`, `0D`, `2D`, then `AD`; unpacking index 5 computes `(0xAD >> 5) & 1 = 1`.

### Result and invariant checks

- Every field round-trips to its input value.
- The five fields occupy eight non-overlapping bits exactly.
- Changing quality to 3 clears `0x30`, writes `0x30`, and yields `0xBD`; all non-quality bits remain unchanged.

### Boundary or failure trace

OR-only replacement fails when a new value needs to clear an old 1. For example, replacing quality `3` with `1` via `word | (1 << 4)` leaves the old high field bit set and still decodes as `3`. Clear-before-OR produces the intended `1`.

## M002 - Carry and overflow are different questions

**Introduced by:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 / M002]]

### Input and conventions

Use eight-bit result storage. `C` is the ninth unsigned bit, `V` reports signed two's-complement range failure, `N` copies result bit 7, and `Z` tests whether the low result byte is zero.

### State-by-state trace

Case 1 - signed overflow without carry:

```text
  01111111  unsigned 127, signed 127
+ 00000001  unsigned   1, signed   1
----------
  10000000  unsigned 128, signed -128

C=0: no ninth bit; unsigned 128 fits
V=1: signed mathematical 128 does not fit [-128,127]
N=1: result high bit is set
Z=0: result is nonzero
```

Case 2 - carry without signed overflow:

```text
  11111111  unsigned 255, signed -1
+ 00000001  unsigned   1, signed  1
----------
1 00000000

C=1: unsigned mathematical 256 needs a ninth bit
V=0: signed mathematical result is valid zero
N=0: low result high bit is clear
Z=1: low result byte is zero
```

Subtraction example:

```text
13 - 5 = 13 + (~5 + 1)
00001101 + 11111011 = 1 00001000 -> low byte 8
```

### Result and invariant checks

- `C` and `V` are independent; neither can replace the other.
- `N` reports the stored sign bit, not whether a signed overflow occurred.
- An eight-bit emulator must retain a wider temporary long enough to compute `C`, then mask the architectural result.

### Boundary or failure trace

`10000000` represents `-128`. Complement-plus-one returns `10000000` because mathematical `+128` cannot fit. At the fixed-bit residue level the pattern is stable; evaluating same-type signed negation in C++ overflows and is undefined.

## Reconstructing each state change

### Decimal to binary and back

For 173, successive quotient/remainder pairs are (86,1), (43,0), (21,1), (10,1), (5,0), (2,1), (1,0), (0,1). The remainders are low bits first, so reverse them to get 10101101. Horner accumulators while reading this result are 1,2,5,10,21,43,86,173. This checks both directions without assuming that the displayed order equals array order.

### Replacing a field while preserving neighbors

Replace quality 2 with 1 in AD:

```text
field width w=2, offset s=4
low mask L=03; positioned mask M=30
eight-bit NOT(M)=CF
clear: AD & CF = 8D
encode new value: (01 & 03) << 4 = 10
write: 8D | 10 = 9D
read back: (9D >> 4) & 03 = 01
outside field: AD & CF = 9D & CF = 8D
```

The last equality proves preservation rather than only checking the new field value.

### Multiplication by selected partial products

Multiply 13 by 11. Start result=0, partial=13, multiplier=11. A low multiplier bit of 1 includes the partial product; shift the multiplier right and the widened partial left between stages.

| Stage | Multiplier | Low bit | Partial | Result after inclusion |
|---|---|---|---|---|
| 0 | 1011 | 1 | 13 | 13 |
| 1 | 0101 | 1 | 26 | 39 |
| 2 | 0010 | 0 | 52 | 39 |
| 3 | 0001 | 1 | 104 | 143 |

Check 13×11=143. The widened partial is essential: if it were retained in a narrow original operand, shifts could lose bits before the accumulator sees them.

### Division by shifting the remainder

Divide binary 1101 (13) by 3, reading dividend bits from most to least significant. At each stage append the next bit to the partial remainder, compare with 3, subtract if possible, and append the quotient decision.

| Incoming bit | Remainder before compare | Subtract 3? | New remainder | Quotient prefix |
|---|---|---|---|---|
| 1 | 1 | no | 1 | 0 |
| 1 | 3 | yes | 0 | 01 |
| 0 | 0 | no | 0 | 010 |
| 1 | 1 | no | 1 | 0100 |

Quotient 4 and remainder 1 satisfy 13=4×3+1, with 0≤1<3.

### C++ expression types and a safe index

For `uint8_t a=250,b=10` on the host used here:

1. Stored operands are eight-bit unsigned values.
2. Integral promotion converts each to int because int represents 0–255.
3. Addition executes as int arithmetic, producing int 260.
4. `auto` keeps int 260; explicit conversion to uint8_t stores residue 4.

For int index=−1 and unsigned size=10, an ordinary mixed-sign comparison may convert −1 to a large unsigned value. `std::cmp_less(-1,10u)` correctly returns true mathematically, but that alone does not authorize indexing. The valid condition is `index>=0 && std::cmp_less(index,size)`.

### Binary subtraction and incoming borrow

Take A=5, B=3. With carry C=1, compute 5+NOT₈(3)+1=5+252+1=258: result=2, carry-out=1 (no borrow). With C=0, compute 257: result=1, since an incoming borrow subtracts one extra. For A=3,B=5,C=1, exact unsigned difference is −2: low result=254, carry-out=0 (borrow). This is the binary SBC convention, independent of any repository implementation claim.

## M003 - Bytes to physical memory

### A matching and mismatching endian decode

| Offset | LE byte | BE byte | LE reader accumulator after byte |
|---|---|---|---|
| 0 | 78 | 12 | 00000078 |
| 1 | 56 | 34 | 00005678 |
| 2 | 34 | 56 | 00345678 |
| 3 | 12 | 78 | 12345678 |

Bytes are hexadecimal. Reading LE bytes as BE produces 78563412; no intra-octet bit reversal occurs. A schema version=1, count=0x12345678, payloadSize=9, written U8/U32-LE/U64-LE, yields:

```text
01 78 56 34 12 09 00 00 00 00 00 00 00
```

Field offsets are 0/1/5, length 13. A typical native record places fields at 0/4/8, size 16. Packing may resemble the schema length but does not define its byte order or version contract.

### Struct layout and an aligned array

Assume member size/alignment pairs 1/1, 8/8, 2/2, 4/4.

| Member | Previous end | Aligned start | Occupied offsets | New end |
|---|---|---|---|---|
| flags | 0 | 0 | 0 | 1 |
| id | 1 | 8 | 8-15 | 16 |
| layer | 16 | 16 | 16-17 | 18 |
| color | 18 | 20 | 20-23 | 24 |

Holes are seven plus two bytes; payload 15, size 24. Reordering id/color/layer/flags gives 0-7/8-11/12-13/14 plus one tail byte: size 16. Verify on the target ABI.

A 1/4/1 record ends at nine but has size 12. At base 0x1000, next object starts 0x100C and its integer at 0x1010. A fictitious stride nine would put that integer at 0x100D, breaking alignment.

### Page translation with an unchanged offset

Assume 4-KiB pages and an allowed resident mapping.

```text
VA                  0x12345ABC
P                   0x1000 = 4096
VPN = VA / P        0x12345
offset = VA % P     0xABC
mapping             VPN 0x12345 -> PFN 0x987
frame start         0x987000
PA = start + offset 0x987ABC
```

Process B may map the same VPN to another PFN; adjacent virtual pages may map distant frames. These are address-space mappings, not endian conversions.

Four bytes starting at offset FFE occupy FFE/FFF in page one and 000/001 in page two. A valid first mapping does not prove the second readable. Four bytes at cache-line offset 62 similarly span two 64-byte lines.

### One member load, separated by mechanism

Assume active at offset zero, IEEE-754 32-bit float at offset four, object VA 0x12345000, valid alignment, and 4-KiB pages.

1. Member offset produces VA 0x12345004.
2. Split into VPN 0x12345 and offset 004.
3. TLB/page-table lookup supplies allowed PFN 0x5A7.
4. PA=0x5A7004; cache/memory supplies bytes.
5. With stated little-endian float representation, 00 00 80 3F corresponds to bits 3F800000 and float 1.0; [[IEEE-754 floating point]] develops why later.

A TLB miss can walk valid tables without OS faulting. A nonresident valid page can fault, acquire backing, and resume. A forbidden access may fail. Real processors overlap this conceptual sequence.

### Ownership and lifetime, not geography

A local vector object commonly occupies a frame; elements occupy its separate dynamic buffer. A copied span points to elements, not vector metadata. Returning that span after destroying the vector dangles it; reallocation can also invalidate views. Keeping an owner alive is necessary but not always sufficient.

Static locals retain storage across calls; automatic const locals need not use read-only mappings; string literals have static storage and must not be modified. Old numeric addresses or remaining bytes do not extend lifetime.

### Fault recovery and copy-on-write

Two mappings share frame F read-only. For a permitted private COW write, OS allocates F2, copies F, changes the writer's mapping to F2 with write permission, and restarts. Not every protection violation is repairable.

**Diagnosis:** absurd decoded value→byte order; second array element misaligned→stride/tail padding; released vector span→lifetime; first-touch latency→fault/commitment; allocation fails amid holes→fragmentation; published file lost after crash→publication versus durability.

[[Supplementary/Derivations#M003 - Byte order, layout, and translation]] · [[Ownership and non-owning views]] · [[Atomic publication and durability]]

## M004 - From fields to values and failure paths

### Encode / decode board trace

For 10.625: integer 10→`1010`; fractional repeated doubling emits 1,0,1; normalize `1010.101` to `1.010101×2³`. Sign 0, stored exponent 130, fraction `0x2A0000`. OR contributions `0x00000000`, `0x41000000`, `0x002A0000` to obtain `0x412A0000`. Little-endian bytes are `00 00 2A 41`.

For `0xC1540000`: sign contribution `0x80000000`, exponent contribution `0x41000000`, fraction `0x00540000`. Restore `1.10101₂`, value $1+1/2+1/8+1/32=1.65625$; scale by 8 and negate → −13.25.

### Boundary corpus and expected words

| Word | Expected class | Exact finite value or meaning |
|---|---|---|
| `00000000` | zero | positive zero |
| `80000000` | zero | negative zero |
| `00000001` | subnormal | $2^{-149}$ |
| `007FFFFF` | subnormal | $2^{-126}-2^{-149}$ |
| `00800000` | normal | $2^{-126}$ |
| `3F7FFFFF` | normal | $1-2^{-24}$ |
| `3F800000` | normal | 1 |
| `3F800001` | normal | $1+2^{-23}$ |
| `7F7FFFFF` | normal | $(2-2^{-23})2^{127}$ |
| `7F800000` | infinity | positive infinity |
| `FF800000` | infinity | negative infinity |
| `7FC00001` | quiet NaN | diagnostic raw payload; not a real value |
| `7F800001` | signaling NaN | classify raw integer; do not use in arithmetic |

Largest finite upward nextafter gives infinity; do not label that difference a finite ULP step. Finite exact reconstruction must preserve zero sign, but generating a generic numerical NaN does not preserve the raw payload.

### Rounding trace

At toy precision three bits, 1.125 lies between retained integers 4 (`1.00`) and 5 (`1.01`): choose 4. At 1.375, choose integer 6 (`1.10`) over 5. Tail `1000…` gives G=1,R=0,T=0; retain even low bit, increment odd. Tail `1010…` gives G=1,R=0,T=1; increment regardless of retained parity.

Actual binary32 tie above 1: exact double value $1+2^{-24}$ lies midway between `3F800000` and `3F800001`. Strict nearest conversion chooses `3F800000`. Next tie $1+3\cdot2^{-24}$ chooses `3F800002`. The lab forces runtime conversion and verifies both.

### Two normalization failures are not the same

| Input | Intermediate trace | Policy/mechanism |
|---|---|---|
| $(10^{-8},0)$ | finite norm near $10^{-8}$; norm ≤ float epsilon | absolute cutoff returns zero despite nonzero representability |
| $(10^{20},10^{20})$ | component squares overflow → infinity; norm infinity; each finite component / infinity → zero | intermediate-range failure |
| Same huge finite input, hypot | range-aware norm near $1.4142\times10^{20}$ | finite norm; no unnecessary squared overflow |

These are source-formula reproductions, not live Kairo API tests.

### Infinity exposes more than an equal-infinity edge case

For the supplied hybrid scalar formula, policy $10\epsilon_{32}$:

| Pair | Difference | Scale / allowed difference | Final result |
|---|---|---|---|
| +inf, +inf | NaN | inf / inf | false: NaN comparison |
| 1, +inf | inf | inf / inf | true: inf ≤ inf |
| +inf, −inf | inf | inf / inf | true: inf ≤ inf |

An exact-equality branch followed by a non-finite rejection branch makes an alternative policy explicit. For finite inputs, widened arithmetic and independently validated absolute/relative tolerances prevent accidental comparison overflow in the float-specific teaching implementation.

The supplied matrix inversion excerpt asserts determinant magnitude > epsilon before its identity fallback. For `diag(1e-4f,1e-4f)`, the threshold fails; debug assertions may stop execution, whereas a build without that assertion may reach the fallback. Neither behavior establishes mathematical singularity.

### Serialization policy trace

`1.0f`→raw `3F800000`→LE `00 00 80 3F`. Negative zero→`80000000`→`00 00 00 80`. A raw quiet-NaN word `7FC00001`→`01 00 C0 7F`. These golden bytes are checked independently of round trips.

Canonicalization at the raw-word level maps either zero to `00000000` and any NaN to chosen `7FC00000`; all other words remain unchanged. This avoids consuming a signaling NaN as a float but deliberately discards its payload/class distinction. No existing asset format was changed.

[[Supplementary/Derivations#M004 - Floating-point fields, spacing, and rounding]] · [[Supplementary/Code Snippets#M004 - Binary interchange inspection and strict rounding laboratory]]

## M005 - Rounding paths and update history

### Runtime witness sheet

| Experiment under strict nearest arithmetic | Expected observation |
|---|---|
| binary32 0.1 | word 3DCCCCCD; exact rational 13421773/2^27 |
| binary64 0.1+0.2 versus 0.3 | words 3FD3333333333334 and 3FD3333333333333 |
| 1.5+0.15625 | align exponents by 3; result 1.65625 |
| 2^24+1 | midpoint absorbed to 2^24 |
| 2^24+2 | next float, exactly 16777218 |
| 1753/1024−1751/1024 | exact 2^-9 |
| stored binary32 1.00000006−1 | exact 2^-23, not desired 6e-8 |
| sqrt(1e16+1)−sqrt(1e16) in double | zero after absorption/rounded roots |
| reciprocal conjugate form | useful nonzero value near 5e-9 |
| max float × 2 | infinity; overflow/inexact flags on tested host |
| min normal × 0.5 | exact subnormal; no underflow/inexact on tested host |
| min subnormal × 0.5 | zero tie; underflow/inexact on tested host |
| FMA witness | separate 0, fused 2^-46 |
| 1e20,−1e20,3.14 regrouped | 3.14f versus zero |
| distributive witness | 1192.0928955078125 versus 1024 |

The integer remainder loop for 1/10 emits 00011, then returns to remainder numerator 2, proving periodicity without using an already-approximate decimal float as the conversion reference.

### Kahan trace at binary32 precision

| Input | Adjusted y | New sum | Compensation |
|---|---|---|---|
| 2^24 | 2^24 | 2^24 | 0 |
| 1 | 1 | 2^24 | −1 |
| 1 | 2 | 2^24+2 | 0 |

Naive loses both ones in this order. Kahan preserves this particular aggregate; it is not a universal order-independent sum. Pairwise on [2^24,1,1] splits one item from two, adds the ones first, and also reaches 2^24+2.

### Precision lost on input cannot be recovered later

Intended component 100000001 becomes binary32 100000000. Promoting that stored float gives exact binary64 100000000, not the intended integer. Directly converting the original integer to double preserves it. The cross-product example must distinguish input loss from later multiplication/subtraction error.

### A minimal repeated-storage witness

Two values $0.5$ and $0.5+2^{-24}$ average to exact midpoint $0.5+2^{-25}$. Float persistent state rounds that mean to $0.5$ under nearest-even. Updating with a third value $0.5+2^{-24}$ then gives a different rounded float than preserving the exact/wider mean through all three samples. Double intermediates in the next update cannot recover the midpoint already discarded. These samples stay inside the laboratory's normalized interval $[-1,1]$.

### Capped weighted mean trace

Cap=2, all observation weights=1:

| Step | Sequence A sample | A distance/weight | Sequence B sample | B distance/weight |
|---|---|---|---|---|
| 1 | 0 | 0 / 1 | 0 | 0 / 1 |
| 2 | 1 | 0.5 / 2 | −1 | −0.5 / 2 |
| 3 | −1 | −0.25 / 2 | 1 | +0.25 / 2 |

These exact results differ by 0.5 with no arithmetic approximation needed. Source-reported Maveb state narrowing is an additional mechanism; this is a toy formula reproduction, not a live volume reconstruction test.

### Discrete decisions

nextafter(2.5,−infinity) rounds to pixel 2 via llround; nextafter(2.5,+infinity) rounds to 3. The exact midpoint rounds to 3 by the halfway-away-from-zero rule. A small perturbation crossing this boundary changes which observation is sampled.

A scalar field value changing sign near zero can change a surface-extraction case. Scalar RMS error, sign-change count, vertex displacement and topology measure different consequences; none was measured on real reconstruction data in this merge.

[[Supplementary/Code Snippets#M005 - Strict arithmetic and state-update laboratory]] · [[Supplementary/Derivations#M005 - Representation error, cancellation, and rounding graphs]]

## Representation choice - One quantity through several contracts

### Exact choices and irreversible approximations

| Quantity/contract | Stored representation | Reconstructed result | What this demonstrates |
|---|---|---|---|
| Signed32,F16, value3.25 | raw212992, word00034000 | exactly3.25 | fixed scale is part of interpretation |
| Signed32,F8, values1.5 and2.25 | raw384 and576; wide product221184 | raw864, value3.375 | product scale must be reduced from256² |
| INT4 group[-7,-3.2,0,2.9,6.8],s=1 | levels[-7,-3,0,3,7] | same levels at scale1 | quantization is lossy before packing |
| Offset nibble codes | [1,5,8,11,15] | unpack identical codes | codes are not two's-complement nibbles |
| Low-first packed bytes | 51 B8 0F | five codes, unused high nibble zero | count is needed to distinguish padding |
| UNORM8 code128 |80 hexadecimal |128/255 | normalized midpoint is not exactly0.5 |
| Binary16 value1.5 |3E00 |exactly1.5 | hidden one and exponent bias reused |
| BF16 tie input3F808000 |3F80 |1.0 | finite nearest-even conversion |
| BF16 next tie3F818000 |3F82 |1.015625 | even choice can round upward |

### Fixed negative rounding trace

At divisor2, numerator−5 has magnitude5=2·2+1. The remainder is exactly half and quotient2 is even; nearest-even gives−2. Numerator−7 gives magnitude7=3·2+1, odd quotient3; increment to4 and restore sign, giving−4. Integer division−7/2 instead gives−3, and arithmetic right shift gives−4. Agreement on one example does not prove identical policies.

### Reconstruct the centroid row

Input[-1,-3,2,4] gives negative sum−4/count2 and positive sum6/count2. Centroids are−2 and3. The sign labels select[-2,-2,3,3]. Reference-minus-reconstruction errors[1,−1,−1,1] square to four ones, so RMSE=1; absolute errors also give MAE=1 and max1.

### The byte image keeps meaning outside the bytes

The continuous story displays every header/payload offset. The golden total is52 bytes:28 header plus24 data. Decode the image at byte-buffer offset1 using a subspan: the schema's offset zero moves with the subspan; no typed pointer alignment is assumed.

Change byte at file offset1E from80 to00. Payload word3F800000 becomes3F000000; value1 becomes0.5. Shape, magic, version and total size remain valid. The declared CRC over complete original bytes changes; parsing success alone did not protect the numeric data.

Raw image words80000000,7F800000,7FC00001,7F800001 round-trip through the word codec unchanged. This tests raw bytes, not signaling-NaN arithmetic or payload-preserving float transport.

### Three failure paths that must not be confused

- A code outside0–15 fails the nibble contract before packing.
- A huge claimed shape fails checked size arithmetic/resource policy before allocation.
- A same-size changed payload can pass structure checks while failing an independent integrity comparison.

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#The complete journey - Encode, compute, store, and investigate|Teach the complete journey]] · [[Supplementary/Code Snippets#Representation codecs and bounded file laboratory|Run the regressions]]
