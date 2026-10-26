# Worked Traces

## Phase 8 - From constraints to a stable SVD contract

Follow one small rank-deficient fit through its geometry, answer, and checks. This supplements [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions|the Phase 8 story]].

Let

$$
A=\begin{bmatrix}3&0\\0&0\end{bmatrix},\qquad
b=\begin{bmatrix}6\\5\end{bmatrix}.
$$

The first row observes $x_1$ with gain three. The second row has zero coefficients, so it cannot produce the requested measurement five. The rank is one, the column space is the horizontal axis, and $b$ lies outside it. The null space is the vertical axis: changing $x_2$ is invisible to the map.

The SVD is already diagonal: $U=V=I$, $\Sigma=A$. The pseudoinverse is $A^+=\operatorname{diag}(1/3,0)$. Therefore $A^+b=(2,0)$. Every $(2,t)$ has the same residual $(0,-5)$; $t=0$ is its minimum-norm choice. This trace separately answers consistency, rank, nullity, residual, and the pseudoinverse policy.

For $k=1$ truncation, $A_1=\operatorname{diag}(3,0)$. Here the input already has rank one, so the approximation is exact. If instead $A=\operatorname{diag}(3,2)$, then keeping only the first singular direction gives $A_1=\operatorname{diag}(3,0)$, spectral error $2$, and Frobenius error $2$. The norm names state which reconstruction discrepancy is being reported.

Now use centered planar samples $(-1,0),(0,0),(1,0)$. Their sample covariance with divisor $N-1$ is $\operatorname{diag}(1,0)$. The first component follows the horizontal line; the zero second eigenvalue says the observed samples have no vertical spread. Add small measurement noise and that zero becomes small, making the rank or PCA cutoff a scale-aware modeling decision.

An SVD routine should return dimensions and status, not just three arrays. Check $\|A-U\Sigma V^T\|$ relative to a declared input scale; check $U^TU$ and $V^TV$ separately, with identity sizes matching full/thin conventions; verify descending nonnegative singular values; enforce a maximum iteration count and surface nonconvergence. Passing these checks establishes those measured properties at the tested scale; it does not certify every singular value or guarantee an accurate solution for an ill-conditioned application.

## Phase 7 - The moving-panel tracebook

Use active right-handed rotations, column vectors, radians in code, and the [[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions#Figure M6 - Follow one point through named spaces|declared illustrative minus-Z projection]] with near 1, far 10 and NDC depth $[-1,1]$.

### One local point reaches a screen-relative description

Let local scale be $(2,1,1)$, orientation a positive-Z quarter turn, translation $(10,0,0)$, and point $p=(1,0,0)$. The camera sits at world $(10,0,5)$ with identity orientation.

| Stage | Value | Check |
|---|---|---|
| Scale local point | $(2,0,0)$ | Scaling occurs in the declared local axes. |
| Rotate | $(0,2,0)$ | Right-hand positive Z maps X toward Y. |
| Translate | $(10,2,0)$ | World location; $M=TRS$. |
| Inverse camera pose | $(0,2,-5)$ | Camera is five units above the world plane and looks along $-Z$. |
| Projection to clip | $(0,2,35/9,5)$ | Keep all four homogeneous coordinates. |
| Perspective divide | $(0,2/5,7/9)$ | Finite nonzero divisor; this point is within the selected clip volume. |

Reconstructing local coordinates undoes translation, then rotation, then scale. Applying the inverse orientation with a guessed untranslated offset would mix frames. The same row-major scalar array could implement every operation if logical access and upload conventions remain correct.

### The tangent and normal take different paths

Take local tangent $u=(1,1,0)$ and local normal $n=(1,-1,0)$. Scale then rotate gives $u'=(-1,2,0)$. Reciprocal-scale then rotate gives $n'=(1,1/2,0)$. Their dot is $-1+1=0$. Normalizing $n'$ for shading changes its length, not the perpendicularity. Neither vector receives translation. A mirrored model additionally needs an explicit winding/orientation policy.

### Diagnose interpolation using a quarter point

Interpolate identity to a positive-Z $120^\circ$ orientation. At $t=1/4$, ideal SLERP gives a $30^\circ$ turn. NLERP normalizes coefficients $(7/8,0,0,\sqrt3/8)$, yielding about $27.795772496^\circ$. The midpoint alone would not expose this speed difference. With endpoints $q$ and $-q$, sign-aligning first avoids a zero midpoint and preserves the common orientation.

### A parent edit invalidates only actual dependencies

For car → door → mirror, editing the door's local transform makes door-local and door-world caches stale, along with mirror-world and derived inverse/normal caches. Mirror-local and car caches remain valid. Reparenting the mirror changes its world dependency even if its local coefficients remain identical. Preserving world pose during reparenting instead requires solving the new local map and checking whether the result still fits the allowed TRS representation.

## Phase 6 - The robot panel tracebook

This cumulative trace extends [[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code|the self-contained Phase 6 volume]]. The numbers are illustrative laboratory inputs, not observations from a real robot or renderer.

### One physical object through six representations

Start with metre-valued workcell points $A=(0,0,0)$, $B=(4,0,0)$, $C=(0,3,0)$ and sensor point $P=(1,1,0)$.

| Stage | Intermediate value | Meaning and check |
|---|---|---|
| Edge subtraction | $e_1=(4,0,0)$, $e_2=(0,3,0)$ | Point differences, not new point locations. |
| Dot and Gram | $g_{11}=16$, $g_{12}=0$, $g_{22}=9$ | Perpendicular but non-unit edge basis. |
| Cross | $e_1\times e_2=(0,0,12)$ | Oriented area vector in square metres. |
| Normalization | $n=(0,0,1)$ | Finite unit front normal; triangle area is $6$. |
| Sensor coordinates | $\beta=1/4$, $\gamma=1/3$, $\alpha=5/12$ | Sum is one and all weights are positive. |
| Attribute interpolation | $(5/12)20+(1/4)40+(1/3)50=35$ | Affine temperature model, not an asserted physical heat solution. |

Move a point $D=(0,0,2)$ above the panel. The area normal dotted with $D-A$ is $24$, yielding tetrahedron volume $4$. Swap $B,C$ and both signs reverse; ordinary area and volume do not.

### A mirror changes orientation, not all measurements

Mirror horizontal coordinates with $R=\operatorname{diag}(-1,1,1)$. New first edge is $(-4,0,0)$ and second edge remains $(0,3,0)$. Their cross becomes $(0,0,-12)$. All edge lengths and all original pairwise dot products remain unchanged. The Gram matrix cannot tell us that the orientation flipped.

The ordinary transformed normal direction $Rn$ is still $(0,0,1)$. The cross-derived oriented normal gains the additional factor $\det(R)=-1$. This is why a mesh pipeline needs a consistent front-face and mirror policy rather than one generic “transform vector” call.

### Nonuniform scaling exposes a hidden normal bug

For tangent $(1,1,0)$ and normal $(1,-1,0)$, double only the horizontal scale. Tangent becomes $(2,1,0)$. A naively transformed normal $(2,-1,0)$ gives dot $3$, not zero. The inverse-transpose normal $(1/2,-1,0)$ gives dot zero. Normalize the latter only after deriving its correct direction.

### A frame can be valid without being continuous

With normal $(0,0,1)$, a least-aligned horizontal helper produces tangent $(0,-1,0)$ and bitangent $(1,0,0)$. All axes are unit and perpendicular, and the frame is positive. Perturb the normal through a helper-selection tie: a different helper can become preferred abruptly. Correct axes do not establish smooth frame evolution. Animated systems must decide whether continuity or a texture reference is required.

### Perspective-correct attribute trace

For two projected endpoints with $w_0=1,w_1=2$, equal screen weights $1/2$ produce reciprocal-weight denominator $1/2+1/4=3/4$. Surface weights become $2/3$ and $1/3$. If endpoint attributes are $0$ and $1$, affine surface interpolation at that screen location gives $1/3$, not the naive $1/2$.

### The first lost quantity can precede every formula

Store intended horizontal coordinates $10^8$ and $10^8+1$ in binary32. Near that scale, one unit is below the local representable spacing, so both can round to the same value. Subtraction then yields zero. Promoting those already-rounded values to binary64 does not recover their difference. Retain a wider global representation or subtract in an appropriately local frame before narrowing.

### Collision difference without an explicit shape

Take intervals $A=[2,4]$ and $B=[3,5]$. In positive direction, the difference support is $4-3=1$. In negative direction, choose $2-5=-3$. The origin lies between them, consistent with overlap. For $A=[0,1]$, $B=[3,4]$, the extrema are $-4,-2$, both negative; the intervals do not overlap. A full higher-dimensional collision algorithm must manage its simplex and stopping policies separately.

### What the executed lab actually demonstrates

The inline scalar program checks the panel weights, area, frame, projection, rejection, reflection, finite-state rejection, huge/tiny scaled normalization, angles, transmission and total internal reflection. It also runs 2,000 deterministic moderate-range cases for decomposition, orthogonality, projection idempotence, reflected length and cross antisymmetry/perpendicularity. None of these tests certify all floating-point inputs, performance, a robust orientation implementation, or human understanding.

## XR - From geometry to sample loss and evidence

The complete explanations and code remain in [[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality|the independent XR story]].

Use this table to follow the quantities from one step to the next:

| Step | Numerical witness | What it shows |
|---|---|---|
| Convert screen geometry to angles | FOV $47.658899^\circ$; average 40.286285 PPD; center 37.936213 PPD | Average and local density differ under the planar projection. |
| Downsample without filtering | Alternating zero/one; take even samples; obtain all zeros | Decimation can turn a pattern into a misleading constant. |
| Filter before downsampling | Average each pair; obtain 0.5 | This declared alternating sequence gives its intended mean. The filter is not an ideal universal cutoff. |
| Analyze aligned frequencies | DFT bins 7 and 29 give one-sided amplitudes 0.3 and 0.1 | The stated amplitude normalization separates the two components. |
| Apply the Gaussian optics model | Gains 0.951850 and 0.291213 at 0.05 and 0.25 cycles per pixel | Higher-frequency contrast is reduced more strongly. |
| Average light, then encode | Linear 0.5 encodes to 0.735357 in sRGB | Encoded 0.5 instead decodes to 0.214041. The arithmetic domain matters. |
| Change allocation with the same true-gaze metric | Fixed weighted MSE 0.00610803; gaze-following 0.00378445 | This is one toy-model result, not a human threshold or saved GPU work. |
| Check threshold algebra | $F\!\left(\lambda(\ln2)^{1/k}\right)=0.75$ | The formula matches the selected criterion for the illustrative parameters. |

### Frame history can be clean and wrong

Follow a moving character:

1. The previous frame sees the character at a pixel.
2. The character moves, exposing the background.
3. Incorrect reprojection brings the old character color into that background.
4. Accumulation reduces random noise but retains a ghost.

A depth or identity mismatch can justify rejecting that history.

A gaze-region change creates a second problem. An old peripheral sample may belong to the correct surface but lack the detail required now.

Lower MSE or lower variance alone does not establish that the reused information is correct.

### An allocation claim and a runtime claim are different

Normalize the original frame time to one:

$$
T_{\mathrm{old}}=0.6+0.4=1.
$$

Only the second component benefits. If its work becomes 70% cheaper:

$$
T_{\mathrm{new}}=0.6+0.4(0.3)=0.72.
$$

The speedup is:

$$
S=\frac1{0.72}\approx1.3889.
$$

The calculation assumes additive, non-overlapping costs. A real GPU may have overlap, bandwidth limits, occupancy changes, or synchronization costs.

A post-filtered full reference does not establish that any original shading work was skipped.

## C++ - Value-transfer and cleanup board traces

Essential code and traces remain inline in [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Trace one payload from decode to cache|the payload journey]] and [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Observe construction, failure and reverse cleanup|the event laboratory]].

| Scenario | Recorded events | Interpretation |
|---|---|---|
| Ordinary constructor body throws | 1,2,3,-2,-1 | Both members completed; complete Whole did not. No destructor-body -3 event. |
| Whole completes, then leaves scope | 1,2,3,-3,-2,-1 | Destructor body precedes reverse member cleanup. |
| Delegating target completes, delegating body throws | 4,5,6,-5,-4 | Complete target established object; cleanup invokes its destructor then member destruction. |

Positive IDs mean construction/body entry, negatives mean destruction. The fixed recorder does not allocate from destructors. Exception traces establish the tested lifetime rules, not a policy for every subsystem's error handling.

### Assignment failure versus successful ownership transfer

Start A with byte7 and B with byte9 in separate buffers. Prepare B=A with an injected allocation failure: B still holds byte9 at the same live buffer, A still byte7, and allocation count is unchanged. Move-construct C from A: C owns byte7, A is documented empty. Copy-construct from empty A: new owner is empty without traversing a null range. Move-assign C into B: B's byte9 allocation is released, B takes byte7, C becomes empty. Self-move B through its alias preserves its value under this particular owner's contract. Reset B to empty and end scope: live allocation count0.

### The aliased update does not behave like two copies

For destination+=source; source*=0.5, starting with distinct source4/destination4 gives destination8/source2. If both parameters designate one float, its state is4→8→4. The second statement sees the first statement's write. Passing scalar values instead creates independent parameter values; no caller mutation is implied.

### Observation is not a mandatory NRVO count

On the recorded Clang C++23 build, the named Probe return observed zero moves ordinarily and one with optional elision disabled. The immovable same-type prvalue result remained valid in both. This differentiates language-required direct construction from a compiler-selected named-local elision. Do not substitute these observed counts for a timing or allocation benchmark.

## C++ - Borrow deadlines and dense-pool identity

The essential explanation remains inline in [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#When a valid borrow stops naming the intended thing|the borrow story]]. This deeper board exercise separates the targets of three paths. Assume a vector with three live Slot objects, enough capacity, and assignments that do not end the destination Slot lifetime.

| Event | First-slot borrow | Last-slot borrow | Entity-11 handle |
|---|---|---|---|
| Initial owners [11,22,33] | Live object representing 11 | Live object representing 33 | Must be checked against registry |
| Move-assign last into first | Same live Slot now represents 33 | Still live until pop; moved-from state | Not a location guarantee |
| Pop last | Live but wrong original association | Referent lifetime ended | Must be invalidated for removal under registry policy |
| Reallocate remaining elements | Old storage path invalidated | Already invalid | Can still identify other live entities if registry validates them |
| Validate entity33 and reacquire | New valid path to current owner33 | Do not reuse old path | Lookup binds identity to current location |

Do not read a moved-from value without its type's specified contract; this example's integer members and Slot assignment make the first-slot result explicit. Not every pool implementation uses this algorithm. The source-reported Kairo anchors remain To verify.

### Direct alias versus converted snapshot

Start `count=10`, bind `const int& live=count` and `const double& snapshot=count`. The first referent is count; the second is a new converted double containing10.0. After `count=50`, live observes50 and snapshot observes10.0. At the end of their enclosing scope the extended double temporary dies; no separate double was owned by count.

### A function call does not renew the deadline

Construct temporary string → bind call parameter → return that same path → bind caller's receiving reference → complete full-expression → destroy temporary. The receiver's longer lifetime is irrelevant. Contrast direct local binding, which specifies extension, with returning `std::string` by value, which establishes an owning result. The complete rule is in [[Temporary lifetime extension]], not “references make temporaries live longer.”

### Cache invalidation without exposed storage

Source3/cache-empty → Get holds mutex, computes9, copies9, unlocks → Set holds mutex, replaces source with4, clears cache, unlocks → Get computes16 and copies it under mutex. The earlier returned9 remains a valid independent snapshot. If Get had exposed a reference, clearing the optional could end that referenced integer's lifetime. Locking only while obtaining the reference does not protect its later use.

## C++ - Build and occupant trace navigation

Essential traces stay inside [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Following one call through the build|the build story]] and [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Following an escaped address through time|the escaped-address story]]. The complete inline laboratory supplies observable constructions/destructions without executing stale access.

| Event | Storage | Object or logical occupant | Valid access |
|---|---|---|---|
| Aligned byte array exists | Reserved through its enclosing scope | Byte storage; no explicit Tracked occupant yet | Construct at a suitable location |
| construct_at(...,7) returns | Same buffer | First Tracked initialized, count1 | Returned pointer can access value7 |
| destroy_at(first) completes | Buffer remains | First lifetime ended, destruction count1 | Do not access its members |
| construct_at(...,8) returns | Same location | Second occupant, construction count2 | Use returned second pointer |
| destroy_at(second) completes | Buffer remains until scope exit | Second lifetime ended, destruction count2 | No further Tracked access |
| Registry slot0 changes generation7→8 | Registry slot retained | Old handle no longer names live occupant | Compare against live registry generation |

This deliberate reuse is not the same as dereferencing a pointer to an automatic object after its storage duration ended. Transparent replacement has specific rules; std::launder is not a universal cure for stale pointers.

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
