# Dependency Map

An edge from prerequisite to dependent means the dependent explanation uses the prerequisite's definition or mechanism. Obsidian's graph shows actual note links; the labeled prerequisite lists in concept notes give those edges their meaning.

## Learning spines

| Prerequisite | Dependent | Why the dependency exists |
|---|---|---|
| [[Physical and logical binary states]] | [[Bit]] | A stable two-state abstraction is the unit being encoded. |
| [[Bit]] | [[Positional notation]] | Positions assign numeric weights to those states. |
| [[Positional notation]] | [[Binary conversion]] | Division and weighted sums reconstruct the same digit expansion. |
| [[Bit significance]] | [[Bit mask]] | A selection must identify which positions belong to it. |
| [[Boolean algebra]] | [[Bitwise AND]], [[Bitwise OR]], [[Bitwise XOR]], [[Bitwise NOT]] | Each word operation applies a one-bit truth table to every lane. |
| [[Bit mask]], [[Shift operations]] | [[Bit field]] | Selection plus normalization extracts a range of bits. |
| [[Bit field]], [[Interpretation contract]] | [[Bit packing]] | Packing must preserve declared ownership, widths, and ordering. |
| [[Machine word]], [[Positional notation]] | [[Unsigned modular arithmetic]] | Finite width imposes the modulus $2^N$. |
| [[Unsigned modular arithmetic]], [[Bitwise NOT]] | [[Two's complement]] | A negative residue is complement-plus-one at the same width. |
| [[Half adder]], [[Carry and borrow]] | [[Full adder]] | Multi-position addition needs an incoming carry. |
| [[Full adder]] | [[Ripple-carry addition]] | Chained carry outputs form the circuit's critical path. |
| [[Two's complement]], [[Full adder]] | [[Carry, overflow, negative, and zero flags]] | The same result bits answer distinct signed and unsigned questions. |
| [[Two's complement]] | [[Subtraction through addition]] | Negation lets subtraction reuse the adder. |
| [[Shift operations]], [[Full adder]] | [[Shift-and-add multiplication]] | Set multiplier bits select weighted partial sums. |
| [[Shift operations]], [[Subtraction through addition]] | [[Binary long division]] | Shifted remainders are compared and reduced by subtraction. |
| [[Fixed-width integer types]] | [[Integral promotion]] | Stored width alone does not establish expression width. |
| [[Integral promotion]] | [[Usual arithmetic conversions]] | Promotion occurs before selection of the common operation type. |
| [[Usual arithmetic conversions]] | [[Mixed signedness]] | Conversion can change a negative operand into an unsigned residue. |
| [[Byte and octet]], [[Bit significance]] | [[Endianness]] | Byte ordering must be distinguished from bit weights. |
| [[Endianness]], [[Fixed-width integer types]] | [[Serialization]] | A file format must define the exact emitted byte sequence. |

## Chunk dependency and pending work

M001 provides representation, masks, shifts, and Boolean rules. M002 uses them to build finite arithmetic and explain language types. M003 develops [[Endianness]], [[Alignment and padding]], and [[Addresses and virtual memory]] into byte codecs, layout, page translation, permissions, and lifetime. M004 develops [[IEEE-754 floating point]] through formats, special classes, spacing and rounding. M005, Parts 77–88, is the next expected source. The connected teaching volume explains these prerequisites directly; graph links support navigation and later depth.

Future connections already have definitions: [[IEEE-754 floating point]], [[Cache lines and memory bandwidth]], [[Atomic operations and write contention]], [[SIMD and GPU data layouts]], [[Quantization and compressed weights]], [[Compiler IR and machine instructions]], and [[Guest and host integer domains]]. Update these same notes when their curriculum parts arrive.

## Navigation

[[Keyword Index]] · [[Graphics Engine Mastery - Continuous Notes]] · [[Supplementary/Foundations]] · [[Supplementary/Derivations]] · [[Supplementary/Code Snippets]] · [[Supplementary/Worked Traces]]

## Updating the graph

For each chunk, define new terms before linking them, add prerequisite reasons, link downstream uses, and back-link the relevant teaching chapter. A shared source link is navigation; a prerequisite link explains learning order. The graph can contain navigation cycles even when the prerequisite relation is acyclic.


## M003 - Placement and memory learning spine

| Prerequisites | Dependent | Reason |
|---|---|---|
| [[Endianness]], [[Shift operations]] | [[Little endian]] | Byte order tells how digit significance maps to sequence positions. Shifts relocate a selected byte or address field to its significance position. |
| [[Endianness]], [[Positional notation]] | [[Big endian]] | Byte order tells how digit significance maps to sequence positions. Base-256 weighted digits give the codec's numeric meaning. |
| [[Endianness]], [[Fixed-width integer types]] | [[Byte swapping and network order]] | Byte order tells how digit significance maps to sequence positions. The representation width fixes byte count and valid shift range. |
| [[Alignment and padding]], [[Fixed-width integer types]] | [[ABI and object layout]] | Address divisibility determines legal placement and layout holes. The representation width fixes byte count and valid shift range. |
| [[Alignment and padding]] | [[Internal and tail padding]] | Address divisibility determines legal placement and layout holes. |
| [[Internal and tail padding]], [[Cache lines and memory bandwidth]] | [[Field reordering and AoS versus SoA]] | The position and amount of unused layout space determine reordering/packing effects. Field adjacency and stride affect useful bytes fetched per access. |
| [[Alignment and padding]], [[Internal and tail padding]], [[Undefined behavior]] | [[Packed structures and misaligned access]] | Address divisibility determines legal placement and layout holes. The position and amount of unused layout space determine reordering/packing effects. Hardware completion does not prove a language-level operation legal. |
| [[Addresses and virtual memory]], [[Storage duration and object lifetime]] | [[Pointer arithmetic and provenance]] | Mappings give numeric locations their process-specific storage identity. Access requires an object and backing storage that are still valid. |
| [[Addresses and virtual memory]], [[Bit mask]], [[Shift operations]] | [[Pages and page offsets]] | Mappings give numeric locations their process-specific storage identity. A low-bit mask isolates the within-page offset. Shifts relocate a selected byte or address field to its significance position. |
| [[Pages and page offsets]], [[Process isolation and page permissions]] | [[Page tables and MMU]] | Page identity is translated while within-page position is preserved. The mapping context and permissions constrain which accesses proceed. |
| [[Page tables and MMU]] | [[TLB]] | Cached or faulting translation starts from the mapping/protection mechanism. |
| [[Addresses and virtual memory]] | [[Process isolation and page permissions]] | Mappings give numeric locations their process-specific storage identity. |
| [[Page tables and MMU]] | [[Page faults and demand paging]] | Cached or faulting translation starts from the mapping/protection mechanism. |
| [[Page faults and demand paging]] | [[Copy-on-write]] | Deferred private copying is implemented through a recoverable write fault. |
| [[Storage duration and object lifetime]], [[Process isolation and page permissions]] | [[Call stack and stack frames]] | Access requires an object and backing storage that are still valid. The mapping context and permissions constrain which accesses proceed. |
| [[Storage duration and object lifetime]], [[Alignment and padding]] | [[Dynamic allocation and heap]] | Access requires an object and backing storage that are still valid. Address divisibility determines legal placement and layout holes. |
| [[Dynamic allocation and heap]] | [[Allocator fragmentation]] | Allocated buffers/blocks supply the storage whose ownership and fragmentation matter. |
| [[Interpretation contract]] | [[Storage duration and object lifetime]] | Representation, type, and lifetime must identify what object exists. |
| [[Storage duration and object lifetime]] | [[Static storage and initialization]] | Access requires an object and backing storage that are still valid. |
| [[Storage duration and object lifetime]], [[Process isolation and page permissions]] | [[Read-only data and code]] | Access requires an object and backing storage that are still valid. The mapping context and permissions constrain which accesses proceed. |
| [[Storage duration and object lifetime]], [[Dynamic allocation and heap]] | [[Ownership and non-owning views]] | Access requires an object and backing storage that are still valid. Allocated buffers/blocks supply the storage whose ownership and fragmentation matter. |
| [[Pages and page offsets]] | [[Virtual and physical contiguity]] | Page identity is translated while within-page position is preserved. |
| [[Page faults and demand paging]], [[Ownership and non-owning views]] | [[Memory-mapped files]] | Deferred private copying is implemented through a recoverable write fault. A mapping or publication pipeline must keep referenced bytes valid. |
| [[Serialization]], [[Ownership and non-owning views]] | [[Atomic publication and durability]] | Publication persists already-specified bytes, rather than defining their format. A mapping or publication pipeline must keep referenced bytes valid. |
| [[Process isolation and page permissions]] | [[ASLR]] | The mapping context and permissions constrain which accesses proceed. |
| [[TLB]], [[Allocator fragmentation]] | [[Huge pages and TLB reach]] | Larger pages change the amount of memory covered by cached translations. Coarser allocation granularity introduces space/availability trade-offs. |
| [[Byte and octet]], [[Bit significance]] | [[Endianness]] | Byte units define both storage offsets and significance digits. Distinguish byte significance from where it is stored. |
| [[Interpretation contract]], [[Fixed-width integer types]], [[Endianness]] | [[Serialization]] | Representation, type, and lifetime must identify what object exists. The representation width fixes byte count and valid shift range. Byte order tells how digit significance maps to sequence positions. |
| [[Byte and octet]], [[Addresses and virtual memory]] | [[Alignment and padding]] | Byte units define both storage offsets and significance digits. Mappings give numeric locations their process-specific storage identity. |
| [[Byte and octet]] | [[Addresses and virtual memory]] | Byte units define both storage offsets and significance digits. |

## M004 - Representation and rounding learning spine

| Prerequisites | Dependent | Why these steps are needed |
|---|---|---|
| [[IEEE-754 floating point]], [[Bit field]] | [[Binary32]] | A 32-bit binary interchange format with one sign bit, eight exponent bits and 23 fraction bits. Normal precision is 24 bits; exponent bias is 127. Word 3F800000 has E=127,F=0 and means 1. |
| [[Binary32]] | [[Binary64]] | A 64-bit binary interchange format with 1/11/52 fields, 53 significant normal bits and bias 1023. Normal exponent range is −1022 through 1023. Word 3FF0000000000000 means 1. |
| [[Binary32]] | [[Exponent bias]] | Normal exponents are encoded as unsigned E=e+B. Binary32 B=127, so exponent 3 stores 130. Field zero and all-ones are reserved; classify before subtracting bias. |
| [[Binary32]], [[Positional notation]] | [[Significand and hidden bit]] | The significand supplies significant digits; the exponent supplies scale. A normal binary value begins 1.f, so its leading one is implied, not stored. Binary32 fraction F contributes F/2^23; subnormals instead begin 0.f. |
| [[Exponent bias]], [[Significand and hidden bit]], [[Hexadecimal]] | [[Floating-point encoding and decoding]] | Encoding chooses fields for a value; decoding classifies fields and reconstructs their meaning. 10.625 normalizes to 1.010101₂×2³, giving 412A0000. C1540000 decodes to −13.25. |
| [[Binary32]] | [[Signed zero]] | Exponent and fraction zero permit either sign. 00000000 and 80000000 compare numerically equal, but signbit and reciprocal distinguish them. Ordinary x<0 does not detect negative zero. |
| [[Binary32]] | [[Floating-point infinities]] | All-one exponent and zero fraction encode signed infinity. This is not maximum finite. Infinity minus itself is NaN; overflow's result also depends on rounding direction. |
| [[Binary32]] | [[NaNs and payloads]] | All-one exponent with nonzero fraction encodes a non-numerical, unordered NaN. Quiet and signaling classes differ; fraction bits can carry payload information. Ordinary NaN equality is false; use isnan. Raw words can be classified without consuming signaling NaNs as floating values. |
| [[Exponent bias]], [[Significand and hidden bit]] | [[Subnormals and gradual underflow]] | Exponent zero and nonzero fraction remove the hidden one and hold the effective exponent at the normal minimum. Binary32 values are F×2^-149, connecting smoothly to 2^-126. Absolute spacing stays fixed while relative precision degrades. |
| [[Significand and hidden bit]] | [[Machine epsilon]] | For these binary types epsilon is upward spacing at 1: 2^-23 for binary32 and 2^-52 for binary64. It is neither minimum positive value nor a physical threshold. |
| [[Machine epsilon]], [[Exponent bias]], [[Subnormals and gradual underflow]] | [[ULP and representable spacing]] | A unit in the last place describes local spacing. Inside normal binade [2^e,2^(e+1)), p-bit precision gives spacing 2^(e-(p-1)). Immediately above 1 the binary32 gap is twice the gap below it. |
| [[ULP and representable spacing]] | [[Rounding to nearest ties to even]] | Choose the nearest representable value; exactly halfway choose the even retained significand integer. At toy precision three bits, 1.125→1 and 1.375→1.5. std::round uses halfway-away-from-zero instead. |
| [[ULP and representable spacing]], [[Rounding to nearest ties to even]] | [[Unit roundoff]] | Under nearest binary rounding the conventional relative bound for normal-range results is u=2^-p, half upward epsilon. For binary32 this is 2^-24. The bound has range/rounding assumptions. |
| [[Boolean algebra]], [[Rounding to nearest ties to even]] | [[Guard round and sticky bits]] | Guard is first discarded bit, round the next, sticky ORs all remaining bits. With retained low bit L, nearest-even increments when G AND (R OR sticky OR L). Sticky summarizes any nonzero tail. |
| [[Rounding to nearest ties to even]] | [[Directed rounding modes]] | Upward means toward positive infinity, downward toward negative infinity, and toward-zero truncates magnitude. Upward rounding of −2.1 to an integer gives −2, not −3. |
| [[Directed rounding modes]], [[NaNs and payloads]] | [[Floating-point environment and compiler modes]] | The environment includes rounding direction and exception status. Runtime mode changes can fail and must restore prior state. Strict compiler settings are needed; fast-math may discard required special-value or environment assumptions. |
| [[IEEE-754 floating point]], [[Subnormals and gradual underflow]] | [[Floating-point exception flags]] | Invalid, divide-by-zero, overflow, underflow and inexact are status conditions, not C++ throw exceptions. Exact subnormal results need not signal underflow. Flag/trap behavior depends on the environment. |
| [[Subnormals and gradual underflow]], [[Floating-point environment and compiler modes]] | [[Flush-to-zero and denormals-are-zero]] | FTZ flushes certain tiny results to zero; DAZ treats subnormal inputs as zero in supporting environments. They change gradual-underflow behavior. Cost and support are target/mode-specific. |
| [[Fixed-width integer types]], [[Interpretation contract]], [[IEEE-754 floating point]] | [[Bit casting and representation]] | A same-size eligible bit_cast transfers representation, not numeric value. On the checked binary32 host, bit_cast<uint32_t>(1.0f)=3F800000, whereas a numeric cast gives 1. Pointer punning is not an equivalent legal operation. |
| [[ULP and representable spacing]], [[Signed zero]], [[NaNs and payloads]] | [[ULP distance policies]] | Distance counts representable steps under a declared ordering/equality policy. The finite-only laboratory collapses both zeros to one key and rejects NaNs/infinities. Across negative minimum subnormal to positive minimum subnormal distance is two. |
| [[Machine epsilon]], [[ULP and representable spacing]], [[Floating-point infinities]], [[NaNs and payloads]] | [[Numerical tolerance policies]] | Tolerance is a task-chosen allowed discrepancy: absolute in original units, relative to scale, or representable-step based. Machine epsilon is not automatically a norm cutoff. Handle non-finite values and validate tolerances before formulas. |
| [[IEEE-754 floating point]], [[Floating-point infinities]] | [[Robust norm and intermediate range]] | A final representable answer can be lost to overflowing intermediates. Squaring 1e20f overflows before a finite norm could be formed. Scale by maximum component or use hypot to avoid unnecessary squared overflow. |
| [[Numerical tolerance policies]] | [[Conditioning and singularity]] | Conditioning measures sensitivity to input perturbation. A=1e-4 I has determinant 1e-8 but condition number one in the 2-norm. Uniform scaling changes determinant magnitude without changing this conditioning measure. |
| [[Signed zero]], [[NaNs and payloads]], [[Serialization]], [[Bit casting and representation]] | [[Floating-point canonicalization]] | Canonicalization deliberately merges encodings: both zeros to positive zero, all NaNs to one quiet pattern. Raw preservation instead keeps sign/payload distinctions. Neither is universally correct; this is a file/hash contract choice. |
| [[Directed rounding modes]], [[Floating-point environment and compiler modes]] | [[Interval arithmetic]] | An interval encloses a possible exact value between lower and upper bounds. Correct outward rounding can preserve containment when the whole algorithm honors necessary assumptions. |

M004 is merged through Part 76. The next expected source is M005, Parts 77–88. Prerequisites are explanatory dependencies, not a requirement to navigate out of the continuous volume.
