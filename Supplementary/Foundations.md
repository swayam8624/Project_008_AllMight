# Foundations

This cumulative supplement retains the per-part definition and coverage ledger. The connected teaching volumes now explain the essential foundations, unfamiliar terms and worked steps directly; readers should not need this ledger to reconstruct a lesson. Concept links provide optional navigation. Examples and clarifications are explanatory material; repository claims retain their evidence status in [[Supplementary/Sources and Code Anchors]].

## M001 - Parts 1-20

| Part | Definition, mechanism, and concrete reconstruction cue |
|---|---|
| 1 | [[Physical and logical binary states]]: a device maps ranges of a physical measurement to 0/1; thresholds and noise margins let imperfect signals represent stable logical values. Storage retains state, while combinational logic calculates from present inputs. |
| 2 | [[Positional notation]]: bit $b_i$ at position $i$ contributes $b_i2^i$. $N$ independent bits provide $2^N$ patterns, whose unsigned values run from 0 through $2^N-1$. |
| 3 | [[Binary conversion]] by Euclidean division: $n=2q+r$, $r\in\{0,1\}$. Save $r$, repeat on $q$, and reverse the saved sequence. For 13, remainders 1,0,1,1 give 1101. Handle zero explicitly. |
| 4 | Weighted decoding sums set-bit weights. Horner decoding repeatedly uses $v\leftarrow2v+b$ from left to right. Reading 1101 yields 1,3,6,13, confirming the same value. |
| 5 | [[Hexadecimal]] has 16 digits; four bits make one nibble and one hex digit. 1010 is A, 1101 is D, so 10101101 is AD. |
| 6 | [[Octal]] has eight digits and groups three bits. Left-pad 10101101 to 010 101 101, giving octal 255. Leading zero in a C++ integer literal requests octal. |
| 7 | [[Byte and octet]]: octet means eight bits; C++ byte width is CHAR_BIT. [[Machine word]] describes a processing convention, not a fixed unit. Exact format widths need explicit types. |
| 8 | [[Bit mask]]: a pattern selects owned positions. A low-width mask is $2^w-1$ because subtracting one from a power of two fills all lower positions with ones. |
| 9 | [[Bitwise AND]] filters: $x\land0=0$, $x\land1=x$. `AD & 07 = 05`. Compare the result with zero for any selected flag, or with the full mask for all selected flags. |
| 10 | [[Bitwise OR]] sets: $x\lor1=1$, $x\lor0=x$. `AD \| 40 = ED`. Repeating the same set operation has no further effect. |
| 11 | [[Bitwise XOR]] toggles: $x\oplus1=\neg x$, $x\oplus0=x$. `AD ^ 08 = A5`; toggling again restores AD. XOR of two states marks differing positions. |
| 12 | [[Bitwise NOT]] inverts all positions at a stated width. Eight-bit NOT(0F)=F0; promoted int-width NOT has more positions. State width before interpreting its value. |
| 13 | [[Shift operations]] relocate bits. Logical right shift zero-fills; arithmetic right shift sign-fills. Left shift zero-fills low positions; discard behavior and numeric meaning depend on width and language rules. |
| 14 | For nonnegative unsigned arithmetic, left shift by $k$ resembles multiplication by $2^k$ and right shift floor division. Truncation changes the exact answer; signed division toward zero differs from arithmetic right shift for negative odd values. |
| 15 | [[Bit field]] extraction normalizes with right shift and isolates with a low mask: `(AD >> 4) & 03 = 2`. Mask-only gives 0x20, still at its original significance. |
| 16 | Insertion clears the destination mask, shifts the valid new value, then ORs it into the cleared word. Preserve outside-mask bits. Replacing quality 2 with 1 in AD produces 9D. |
| 17 | [[Bit packing]] combines disjoint, range-checked fields. OR contributions 05,08,20,00,80 to obtain AD. Packing saves payload bytes but introduces extraction and update work. |
| 18 | Unpacking reverses the declared schema. For LSB-first bits use byte $\lfloor i/8\rfloor$ and offset $i\bmod8$. Check the requested count, source capacity, and unused padding before indexing. |
| 19 | [[Boolean algebra]] defines false/true operations by truth tables. [[Logical versus bitwise operators]] differ in result domain; built-in logical AND/OR also [[Short-circuit evaluation\|skip unnecessary operands]]. |
| 20 | [[De Morgan's laws]] swap AND and OR when negation crosses the operator. Enumerate four inputs to prove both identities; keep a fixed width for the bitwise versions. |

## M002 - Parts 21-40

| Part | Definition, mechanism, and concrete reconstruction cue |
|---|---|
| 21 | Unsigned $N$-bit range is $[0,2^N-1]$. Distinguish state count 256 from maximum eight-bit value 255. |
| 22 | [[Signed magnitude]] uses one sign bit and $N-1$ magnitude bits. It has two zeros and requires sign-aware magnitude arithmetic. Eight-bit −5 is 10000101. |
| 23 | [[One's complement]] encodes negatives by bit inversion. −5 is 11111010. All ones is negative zero; addition returns a final carry to the low end as end-around carry. |
| 24 | [[Two's complement]] uses the negative residue $2^N-x$ and signed top-bit weight $-2^{N-1}$. Decode AD as 173−256=−83. The same adder supports positive and negative residues. |
| 25 | Negation follows $2^N-x=(2^N-1-x)+1$. The first term is the width-limited complement, so −13 is NOT(00001101)+1=11110011. |
| 26 | Negating zero gives all ones plus one, leaving low bits zero. Two's complement has exactly one zero and assigns every pattern a distinct signed integer. |
| 27 | [[Minimum signed value]] follows the asymmetric range $[-2^{N-1},2^{N-1}-1]$. For int minimum, same-type negation, abs, or division by −1 cannot represent the positive result. Narrow storage may promote first. |
| 28 | [[Unsigned modular arithmetic]] retains residues modulo $2^N$. 250+10 becomes 4 when narrowed to eight bits; on the usual host the original byte addition expression is int 260. |
| 29 | [[Signed integer overflow]] is [[Undefined behavior]] at the operation type's width. Check bounds before evaluating, or use a type proved large enough. Evaluating overflow and checking afterward is too late. |
| 30 | Unsigned 0−1 wraps to maximum; going below signed minimum is signed overflow. A size_t loop condition `i>=0` never becomes false. Prefer reverse iteration or a decrement-before-index pattern with a correct termination guard. |
| 31 | [[Carry and borrow]] move arithmetic information between digit positions. Four-bit 13+7 gives low 4 with carry 1. Unsigned carry differs from signed range failure. Borrow conventions depend on architecture. |
| 32 | [[Half adder]]: sum=A XOR B, carry=A AND B. [[Full adder]] adds carry-in; sum is three-way parity and carry is the majority of three inputs. A majority function is true when at least two inputs are true. |
| 33 | [[Ripple-carry addition]] chains full adders. Worst-case circuit delay grows with width; fixed-width software addition remains constant work. Generate/propagate terms permit [[Carry-lookahead and parallel-prefix adders\|parallel carry networks]]. |
| 34 | [[Subtraction through addition]] uses $A+\operatorname{NOT}_N(B)+1$. Binary 6502-style SBC instead includes C: $A-B-(1-C)$, so C=1 means no borrow. |
| 35 | [[Shift-and-add multiplication]] sums one shifted multiplicand for every set multiplier bit. 13×11=13+26+104=143. Both the shifting partial value and accumulator must hold the required wider range. |
| 36 | [[Binary long division]] shifts dividend bits into a partial remainder, then compares and conditionally subtracts the divisor. Check $D=Qd+R$ and $0\le R<d$. For 13/3, Q=4,R=1. Zero division and int-min/−1 are invalid. |
| 37 | [[Saturating arithmetic]] clamps a widened answer to endpoints: 250+20 becomes 255. Signed saturation is not associative, so evaluation order matters. Do not prematurely clamp an HDR intermediate. |
| 38 | [[Fixed-width integer types]] define exact widths when available. Least and fast families promise at least the width, with different goals. size_t is for sizes, ptrdiff_t for differences; plain char signedness varies. |
| 39 | [[Integral promotion]] occurs first; [[Usual arithmetic conversions]] then select a common type. Track stored type, promoted type, operation type, and destination conversion separately. `~uint8_t{0}` is usually int −1. |
| 40 | [[Mixed signedness]] can convert negative operands to large unsigned values. Use consistent semantic types or std::cmp_* helpers; valid indexing still requires nonnegative and less-than-size conditions. std::in_range checks conversion representability. |

## Notation and validity

$N$ is carrier width, $i$ a bit position, $w$ field width, $s$ offset, and $b_i$ a 0/1 digit. $\land$, $\lor$, $\oplus$, and $\neg$ mean AND, OR, XOR, and NOT; integer vectors require a stated width. $\ll$ and $\gg$ denote shifts. Modulo records a residue; floor rounds downward. A bijection pairs every element of one set with exactly one element of the other. An invariant is a property that must remain true across steps. An ALU is an arithmetic and logic unit.

Keep number semantics separate from executable C++ operators. A word-level formula assumes a fixed unsigned carrier; C++ built-in operators act after promotion. For C++23, signed right shift rounds toward negative infinity. Claims about signed overflowing addition remain subject to undefined behavior.

## Why these dependencies matter

Representation supplies the weights needed for masks. Boolean operators supply the gates needed for adders. Masks and shifts supply the field operations needed for compact data. Finite width supplies the modular model needed for two's complement. Promotions and conversions determine the domain in which C++ actually performs each operation. See [[Dependency Map]] and each concept's prerequisites before reconstructing a later mechanism.

## M003 - Parts 41-60

| Part | Definition, mechanism, and concrete reconstruction cue |
|---|---|
| 41 | [[Endianness]] orders significance bytes at increasing addresses. It does not change the integer value or reverse bits inside each octet. Separate significance, address traversal, and intra-byte packing order. |
| 42 | [[Little endian]] puts the least-significant byte first: 0x12345678 produces 78 56 34 12. Extract byte i by unsigned right shift by 8i, then narrow to eight bits; reconstruct by shifting each byte into its significance position. |
| 43 | [[Big endian]] puts the most-significant byte first: 12 34 56 78. Read left-to-right with value←256×value+byte: M001 Horner decoding in base 256. |
| 44 | [[Byte swapping and network order]]: swapping reverses octet significance; host/network conversion is a no-op when orders already match. Classic Internet fields often use big endian, but each protocol defines its own contract. [[Serialization]] fixes widths/order and validates lengths without native padding. |
| 45 | [[Alignment and padding]]: alignment A restricts starting addresses to multiples of A. C++ alignof reports the requirement; alignas requests a supported stronger alignment. Align-up needs nonzero A and overflow checks; an aligned numeric offset is not automatically a live pointer. |
| 46 | [[Internal and tail padding]]: internal padding precedes a member to meet its alignment. Under 1/4/1 member alignments, byte–uint32–byte uses offsets 0/4/8, with three internal padding bytes. [[ABI and object layout]] determines actual offsets. |
| 47 | Tail padding rounds complete-object size for aligned array stride. The same example ends at byte nine and rounds to size 12; the next object's four-aligned member remains aligned. Never infer sizeof solely by summing payload sizes. |
| 48 | [[Field reordering and AoS versus SoA]]: order by decreasing alignment can reduce holes. A 1/8/1/4 example commonly shrinks 24→16 bytes. Measure sizeof/alignof/offsetof; ABI compatibility and hot/cold access patterns can outweigh smaller size. AoS stores complete records; SoA has one array per field. |
| 49 | [[Packed structures and misaligned access]]: packing extensions can remove holes but weaken member alignment. They do not define endianness, versions, or lifetime. A packed native object differs from a specified wire format. |
| 50 | Misalignment can cross cache-line/page boundaries, slow access, or fault depending on instruction/platform. C++ access still needs a suitably aligned live object. Reconstruct bytes or copy representation into an existing aligned trivially copyable object; copying alone does not normalize endianness. |
| 51 | [[Pointer arithmetic and provenance]]: address is a location identifier; a C++ pointer is a constrained typed access path. Within an array, p+i advances i×sizeof(T) bytes; one-past can be formed but not dereferenced. Numeric addresses alone do not establish access rights. |
| 52 | [[Addresses and virtual memory]] translates process-visible addresses to physical addresses. A contiguous vector buffer can span scattered frames. [[Virtual and physical contiguity]] distinguishes address-space adjacency from backing placement. |
| 53 | [[Process isolation and page permissions]]: each address space has its own mapping context. VA 0x400000 can map to frame 27 in A and frame 913 in B. Shared mappings may select the same frame; read/write/execute/user permissions constrain access. |
| 54 | [[Pages and page offsets]]: a virtual page is an address block; a frame is corresponding physical storage. For P=2^k, VPN=floor(VA/P), offset=VA mod P. The 4-KiB example uses k=12; actual page size depends on platform/configuration, not universally 4096. |
| 55 | [[Page tables and MMU]]: entries supply frame identity, permissions, and state; MMU is the translation/protection mechanism. The [[TLB]] caches translations, not data. A miss may resolve through page-table walking without a page fault. |
| 56 | [[Page faults and demand paging]]: OS intervention is needed under current mapping state. Valid demand loading/zero-fill may install a frame and retry; [[Copy-on-write]] may copy on first write. Unmapped/forbidden access may fail. A fault does not necessarily imply disk I/O. |
| 57 | [[Call stack and stack frames]]: calls conventionally use LIFO frames for return state, registers, spills, and some locals. Downward growth is a convention; not every local has a stack slot. Returning a dead-local pointer violates [[Storage duration and object lifetime]], even if bytes remain. |
| 58 | [[Dynamic allocation and heap]] subdivides OS regions into variable-lifetime blocks. [[Allocator fragmentation]] distinguishes slack within allocated blocks (37 requested in a 48-byte class leaves 11) from separated free holes unable to satisfy one request. Allocation cost differs from access latency. |
| 59 | [[Static storage and initialization]]: namespace/static objects have program-duration storage. Initialized writable data commonly uses data-like regions; zero-initialized storage uses BSS-like zero-fill without literal executable zero payload. Dynamic initialization and cross-unit ordering need later treatment. |
| 60 | [[Read-only data and code]]: text commonly contains instructions, read-only regions constants. Writable-or-executable policy limits writable executable mappings. Modifying a string literal is undefined behavior; const automatic objects are not forced into read-only segments. Segment names/layout are implementation choices. |

### Bridges and distinctions that must not be omitted

- [[Ownership and non-owning views]]: vector buffer ownership differs from where its object lives. A span neither copies nor extends buffer lifetime; destruction or invalidating reallocation can dangle it.
- [[Memory-mapped files]]: a file-backed virtual mapping is not the file itself; lazy faulting and synchronization matter.
- [[Atomic publication and durability]]: replacement concerns namespace visibility. Crash durability requires a platform/filesystem flush protocol; atomic publication alone does not prove it.
- [[ASLR]] randomizes mapping locations; a textbook process diagram is not a layout guarantee.
- [[Huge pages and TLB reach]]: larger pages cover more bytes per translation but trade allocation flexibility and granularity. One GiB contains 262,144 4-KiB pages versus 512 2-MiB pages; no universal speedup follows.
- Thread storage duration exists alongside automatic, static, and dynamic duration; thread-local storage is not simply the call stack. Storage duration governs available storage; lifetime governs the live object within it.

**Clarifications:** the layout examples assume stated ABI rules; reordering may break a published ABI. Bounds and lifetime remain necessary even with valid mappings/alignment. Wording cross-checked against the C++ draft [alignment](https://eel.is/c++draft/basic.align), [storage duration](https://eel.is/c++draft/basic.stc), and [endianness](https://eel.is/c++draft/bit.endian) sections.

## M004 - Parts 61-76

| Part | Definition, mechanism, and concrete reconstruction cue |
|---|---|
| 61 | [[IEEE-754 floating point]]: finite nonuniform representation plus explicitly specified rounding/special-value behavior; formats do not make every C++ float binary32. |
| 62 | [[Binary32]]: 1/8/23 stored fields, 24-bit normal precision and bias 127; 1.0 has word 3F800000. |
| 63 | [[Binary64]]: 1/11/52 stored fields, 53-bit normal precision and bias 1023; more precision is distinct from more exponent range. |
| 64 | [[Signed zero]]: floating sign is separate from magnitude; signbit sees negative zero whereas x<0 does not. |
| 65 | [[Exponent bias]]: normal stored exponent E=e+B; E=130 decodes to e=3 for binary32; classify reserved fields first. |
| 66 | [[Significand and hidden bit]]: normal leading binary 1 is implicit; 23 fraction bits plus it give 24 significant bits. |
| 67 | [[Floating-point encoding and decoding]]: 10.625=1.010101₂×2³ gives S=0,E=130,F=2A0000, word 412A0000. |
| 68 | [[Floating-point encoding and decoding]]: C1540000 has S=1,E=130,F=540000; reconstruct −1.65625×8=−13.25. |
| 69 | [[Signed zero]]: 00000000 and 80000000 compare numerically equal but have distinct representations/reciprocals. |
| 70 | [[Floating-point infinities]]: all-one exponent with zero fraction; distinct from maximum finite and dependent on sign/rounding in overflow. |
| 71 | [[NaNs and payloads]]: all-one exponent with nonzero fraction; unordered comparisons, quiet/signaling distinction, payload preservation caveats. |
| 72 | [[Subnormals and gradual underflow]]: E=0 uses no hidden one; binary32 x=(-1)^S F×2^-149, filling the normal-to-zero gap. |
| 73 | [[Machine epsilon]]: spacing above 1, 2^-23 for binary32, not min(), denorm_min(), or a universal tolerance. |
| 74 | [[ULP and representable spacing]]: normal binade spacing is 2^(e-23) for binary32; 1 has different immediate lower/upper gaps. |
| 75 | [[Rounding to nearest ties to even]]: choose nearest neighbor; exact halfway picks even retained significand; guard/round/sticky summarize discarded bits. |
| 76 | [[Directed rounding modes]]: upward, downward and toward-zero are signed directions; runtime environment changes must be checked and restored. |

The continuous chapter contains these definitions and essential traces directly. Numerical reproductions establish supplied-formula behavior, not current repository implementation; see [[Supplementary/Sources and Code Anchors]].

## M005 - Parts 77-88

| Part | Definition, mechanism, and concrete reconstruction cue |
|---|---|
| 77 | [[Terminating fractions and dyadic rationals]]: a reduced fraction terminates in binary iff its denominator is a power of two; 0.1f stores 13421773/2^27, not 1/10. |
| 78 | [[Floating-point addition and absorption]]: align exponents, preserve guard/round/sticky, normalize and round; 2^24+1 is a nearest-even tie that stores 2^24. |
| 79 | [[Cancellation and loss of significance]]: nearly equal operands expose prior error relative to a small difference; sensitivity factor (|a|+|b|)/|a-b|. |
| 80 | [[Sterbenz lemma]]: nearby nonnegative stored floats satisfying x/2≤y≤2x have an exact representable difference under suitable gradual-underflow rules; input uncertainty remains. |
| 81 | [[Floating-point exception flags]]: max×2 overflows; min-normal/2 is exact subnormal, while min-subnormal/2 rounds to zero with tiny/inexact behavior in the tested environment. |
| 82 | [[Absolute and relative error]]: absolute discrepancy carries units; relative discrepancy divides by nonzero reference magnitude. Distinguish forward/backward error and stability from conditioning. |
| 83 | [[Numerical tolerance policies]]: separate absolute and relative tolerances; validate them and non-finite inputs. Numeric exact equality is not representation equality. |
| 84 | [[Numerical tolerance policies]]: a global epsilon mixes units, scales, algorithmic error and conditioning; approximate closeness is nontransitive. |
| 85 | [[Fused multiply-add]]: one-rounding fl(ab+c) differs from fl(fl(ab)+c); the (1+2^-23)^2 witness yields 2^-46 rather than zero. |
| 86 | [[Floating-point reassociation]]: (a+b)+c and a+(b+c) can differ; naive, compensated and fixed pairwise reductions have different contracts. |
| 87 | [[Floating-point reassociation]]: distributive graphs differ: with a=1e10,b=1+2^-23,c=-1, factored result 1192.0928955078125 versus unfused expanded 1024. |
| 88 | [[Numerical reproducibility]]: precision, ordering, FMA, environment and compiler/math-library settings matter. Atomicity does not fix global order; capped TSDF updates are mathematically history-dependent. |

All essential explanations and traces are in the continuous chapter. Repository findings remain supplied-source claims; no live engine experiment is implied.
## M006 - Parts 89-100

| Part | Definition, mechanism, and concrete reconstruction cue |
|---|---|
| 89 | [[Fixed-point representation]]: a signed raw integer I at scale S represents I/S; F=16 gives step 2^-16 and raw 212992 for 3.25. |
| 90 | [[Fixed-point range and resolution]]: require 2^-F≤requested step and M·2^F≤2^(N-1)-1. A 32-bit domain ±1e6 with step≤0.001 permits F=10 or 11. |
| 91 | [[Fixed-point rescaling]]: multiply at scale S², then round/divide by S; division uses A·S/B. Prove intermediate width, handle negative ties and zero divisors, and check destination range. |
| 92 | [[Arithmetic policies]] remain necessary for fixed point. Nearest unclipped absolute error≤1/(2S); near-zero relative error can be large. Controlled integer arithmetic can improve repeatability, not make every operation associative. |
| 93 | [[Affine quantization and zero point]] maps finite values to codes and reconstructs s(q-z). Groupwise scales cost metadata; source-shaped offset INT4 codes [1,5,8,11,15] pack to 51 B8 0F. One-bit row centroids are not automatically ±1. |
| 94 | [[UNORM and SNORM]]: UNORM8 reconstructs q/255; SNORM8 clamps q/127 at −1. Quantized normal components can require safe renormalization; sRGB is a separate interpretation. |
| 95 | [[Binary16]] uses 1/5/10 fields, bias15, 11 normal precision bits, maximum65504, normal minimum2^-14, subnormal minimum2^-24. Word3E00 encodes1.5. |
| 96 | [[Bfloat16]] uses1/8/7 fields, bias127, 8 normal precision bits; it trades detail for wide exponent range. Nearest-even conversion must classify NaNs instead of discarding their low payload into infinity. |
| 97 | [[Bit casting and representation]] transfers suitable initialized object representation; numeric cast converts a value. Eligibility does not solve byte order, arbitrary valid representations, alignment or mapped-object lifetime. |
| 98 | [[Binary format contracts]] declare widths/order/representation/version; [[Checked binary parsing]] validates products, subtracted bounds, exact lengths and resource limits. Magic/version are not integrity or authenticity. |
| 99 | [[Checksums CRCs and hashes]] summarize specified content. Additive sums miss byte reordering; CRC requires named parameters; a cryptographic digest needs a trusted reference, MAC or signature for an adversarial trust claim. |
| 100 | [[Memory dump interpretation]] decodes by offset/schema/byte order. The 2×3 demonstration has a28-byte header and24-byte binary32 payload; identical bytes have different meanings under integer, float or character interpretations. |

### Source-to-story navigation

Source numbering is an audit ledger, not lesson segmentation. The complete teaching narrative reorganizes all six accepted sources by problem and prerequisite.

| Source material | Teaching position |
|---|---|
| Binary states, bases, unsigned/signed encodings | [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|Interpretation]] |
| Boolean rules, masks, shifts and packed fields | [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Bits become decisions and compact fields|Decisions and fields]] |
| Adders, integer algorithms, range policies and language conversions | [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|Finite arithmetic]] |
| IEEE fields, special classes and representable spacing | [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|Floating representation]] |
| Rounding paths, cancellation, error and reproducibility | [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|Arithmetic uncertainty]] |
| Fixed point, FP16/BF16, quantizers and normalized formats | [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Representation choices]] |
| Addresses, layout, page translation and storage lifetime | [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|Placement]] |
| Bit transfer, schemas, checked parsing, integrity and dumps | [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Bytes become a durable agreement|External bytes]] |
