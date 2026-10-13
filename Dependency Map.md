# Dependency Map

## Phase 6 - From movement to reliable geometry

This spine integrates Parts 501–600 without assuming the unreceived Parts 148–500 are complete. Shared numeric and C++ nodes supply explicit prerequisites; independent XR material remains a separate series. Concept notes contain reciprocal downstream links. Read this table from prerequisite to dependent.

| Prerequisites | Dependent | Why the dependency exists |
|---|---|---|
| [[Scalars fields and real-number models]] | [[Vectors coordinates and affine points]] | Field operations define which scalar multipliers the vector laws may use. |
| [[Vectors coordinates and affine points]] | [[Vector spaces subspaces and span]] | Point, displacement and coordinate roles determine which geometric operations are meaningful. |
| [[Vector spaces subspaces and span]] | [[Basis coordinates and dimension]] | Reachability and closure establish the spanning and independence questions. |
| [[Basis coordinates and dimension]] | [[Norms lengths and induced distances]] | An ordered basis fixes coordinate meaning and the space whose geometry is measured. |
| [[Norms lengths and induced distances]], [[Robust norm and intermediate range]] | [[Unit directions and safe normalization]] | Lengths and their domains are needed before unit directions or angular measurements. Range-safe scaling avoids losing a valid direction to unnecessary squared intermediates. |
| [[Norms lengths and induced distances]] | [[Dot products and orthogonality]] | Lengths and their domains are needed before unit directions or angular measurements. |
| [[Dot products and orthogonality]] | [[Projection rejection and scalar components]] | Inner-product measurements supply the perpendicularity and alignment constraints. |
| [[Basis coordinates and dimension]], [[Dot products and orthogonality]] | [[Gram matrices and coordinate metrics]] | An ordered basis fixes coordinate meaning and the space whose geometry is measured. Inner-product measurements supply the perpendicularity and alignment constraints. |
| [[Scalars fields and real-number models]] | [[Complex numbers and planar rotations]] | Field operations define which scalar multipliers the vector laws may use. |
| [[Dot products and orthogonality]] | [[Cross products and oriented area]] | Inner-product measurements supply the perpendicularity and alignment constraints. |
| [[Cross products and oriented area]] | [[Coordinate handedness and orientation]] | Oriented area supplies face normals, frame signs and geometric determinants. |
| [[Cross products and oriented area]], [[Dot products and orthogonality]] | [[Triple products and signed volume]] | Oriented area supplies face normals, frame signs and geometric determinants. Inner-product measurements supply the perpendicularity and alignment constraints. |
| [[Cross products and oriented area]], [[Unit directions and safe normalization]] | [[Robust vector angles]] | Oriented area supplies face normals, frame signs and geometric determinants. Validated finite unit directions establish the preconditions for simplified geometry formulas. |
| [[Cross products and oriented area]], [[Unit directions and safe normalization]], [[Projection rejection and scalar components]] | [[Surface tangents normals and bitangents]] | Oriented area supplies face normals, frame signs and geometric determinants. Validated finite unit directions establish the preconditions for simplified geometry formulas. Removing an unwanted component creates residuals, tangent axes and mirror decompositions. |
| [[Surface tangents normals and bitangents]], [[Gram matrices and coordinate metrics]] | [[Orthonormal frames and Gram-Schmidt]] | The surface tangent plane gives the axes that orthogonalization must preserve. Pairwise basis measurements define the systems and metric used in reconstruction. |
| [[Projection rejection and scalar components]], [[Unit directions and safe normalization]] | [[Reflection refraction and shading directions]] | Removing an unwanted component creates residuals, tangent axes and mirror decompositions. Validated finite unit directions establish the preconditions for simplified geometry formulas. |
| [[Gram matrices and coordinate metrics]], [[Vectors coordinates and affine points]] | [[Barycentric coordinates and interpolation]] | Pairwise basis measurements define the systems and metric used in reconstruction. Point, displacement and coordinate roles determine which geometric operations are meaningful. |
| [[Cross products and oriented area]], [[Dot and cross-product rounding]] | [[Robust geometric predicates]] | Oriented area supplies face normals, frame signs and geometric determinants. Cancellation and rounded inputs explain when a determinant sign or finite measurement is uncertain. |
| [[Barycentric coordinates and interpolation]] | [[Convex combinations and convex hulls]] | Affine weights establish origin-independent combinations and inside-hull restrictions. |
| [[Convex combinations and convex hulls]], [[Dot products and orthogonality]] | [[Minkowski sums and support mappings]] | Convex set geometry is the domain for hull support and collision constructions. Inner-product measurements supply the perpendicularity and alignment constraints. |
| [[Vectors coordinates and affine points]] | [[Homogeneous coordinates and perspective division]] | Point, displacement and coordinate roles determine which geometric operations are meaningful. |
| [[Basis coordinates and dimension]], [[Dot products and orthogonality]] | [[Covectors linear functionals and normal transforms]] | An ordered basis fixes coordinate meaning and the space whose geometry is measured. Inner-product measurements supply the perpendicularity and alignment constraints. |
| [[Cross products and oriented area]], [[Triple products and signed volume]] | [[Exterior products and geometric algebra]] | Oriented area supplies face normals, frame signs and geometric determinants. Oriented volume connects exterior grades and determinant-based geometry. |
| [[Vectors coordinates and affine points]], [[Named members versus array elements]] | [[Vector value types and component access]] | Point, displacement and coordinate roles determine which geometric operations are meaningful. Array-relative pointer rules constrain safe named-component access. |
| [[Vector value types and component access]], [[Alignment and padding]], [[SIMD and GPU data layouts]] | [[Vector storage SIMD and boundary layouts]] | Primitive representation and access rules support layout choices and semantic wrappers. Object alignment and array stride constrain storage and boundary conversions. Lane access and external interface layouts require separate storage contracts. |
| [[Numerical tolerance policies]], [[Dot and cross-product rounding]] | [[Vector tolerances and finite-state diagnostics]] | A declared scale and domain make closeness and rejection thresholds meaningful. Cancellation and rounded inputs explain when a determinant sign or finite measurement is uncertain. |
| [[Vectors coordinates and affine points]], [[Unit directions and safe normalization]], [[Vector value types and component access]] | [[Coordinate-space types and validated invariants]] | Point, displacement and coordinate roles determine which geometric operations are meaningful. Validated finite unit directions establish the preconditions for simplified geometry formulas. Primitive representation and access rules support layout choices and semantic wrappers. |

## Independent XR learning spine

XR01 is outside the3,000-part progression. Physical interpretation, angular geometry, signals, reconstruction and experimental evidence connect within this separate series and to shared foundations.

| Prerequisites | Dependent | Why the dependency exists |
|---|---|---|
| [[Light spectra and radiometry]] | [[Reflectance and rendering equation]] | The light quantity and units must be established before its interaction or visual effect is modeled. |
| [[Light spectra and radiometry]] | [[Retinal encoding and receptive fields]] | The light quantity and units must be established before its interaction or visual effect is modeled. |
| [[Light spectra and radiometry]] | [[Optical focus and binocular conflict]] | The light quantity and units must be established before its interaction or visual effect is modeled. |
| [[Optical focus and binocular conflict]] | [[Visual angle and calibrated rays]] | Optical image formation determines the direction/focus assumptions used by geometry and binocular analysis. |
| [[Visual angle and calibrated rays]], [[Retinal encoding and receptive fields]] | [[Visual acuity and eccentricity]] | Angular coordinates make sampling density and gaze-relative location physically meaningful. |
| [[Retinal encoding and receptive fields]] | [[Contrast sensitivity adaptation and masking]] | Spatial/contrast-sensitive neural responses motivate task-dependent visual models. |
| [[Visual angle and calibrated rays]] | [[Eye movements and display-time gaze]] | Angular coordinates make sampling density and gaze-relative location physically meaningful. |
| [[Interpretation contract]] | [[Continuous signals and discrete samples]] | A numeric sample requires a domain, units and encoding convention. |
| [[Continuous signals and discrete samples]] | [[Fourier analysis and spectral phase]] | Filtering/frequency analysis operates on a declared domain and its samples. |
| [[Fourier analysis and spectral phase]] | [[Sampling aliasing and reconstruction]] | Frequency and phase reveal transfer, sampling ambiguity and structural loss. |
| [[Continuous signals and discrete samples]], [[Fourier analysis and spectral phase]] | [[Convolution and filter footprints]] | Filtering/frequency analysis operates on a declared domain and its samples. |
| [[Optical focus and binocular conflict]], [[Fourier analysis and spectral phase]], [[Convolution and filter footprints]] | [[PSF OTF and MTF]] | Optical image formation determines the direction/focus assumptions used by geometry and binocular analysis. |
| [[Convolution and filter footprints]], [[Sampling aliasing and reconstruction]] | [[Multiscale texture filtering]] | A weighting footprint establishes the filtered response, boundary effects and frequency transfer. |
| [[Sampling aliasing and reconstruction]], [[Eye movements and display-time gaze]] | [[Spatiotemporal sampling and retinal motion]] | Allocation/filtering must respect what reduced samples can distinguish. |
| [[Sampling aliasing and reconstruction]], [[Multiscale texture filtering]] | [[Antialiasing and sample allocation]] | Allocation/filtering must respect what reduced samples can distinguish. |
| [[Spatiotemporal sampling and retinal motion]], [[Antialiasing and sample allocation]] | [[Temporal reprojection and history validation]] | Temporal reuse must respect changing scene and retinal observations. |
| [[Light spectra and radiometry]], [[Interpretation contract]] | [[Color encoding and linear-light operations]] | The light quantity and units must be established before its interaction or visual effect is modeled. |
| [[Contrast sensitivity adaptation and masking]], [[Fourier analysis and spectral phase]], [[Color encoding and linear-light operations]] | [[Perceptual metrics and display calibration]] | Visibility judgments require sensitivity and stimulus-context definitions. |
| [[Contrast sensitivity adaptation and masking]] | [[Psychophysics and threshold inference]] | Visibility judgments require sensitivity and stimulus-context definitions. |
| [[Visual acuity and eccentricity]], [[Temporal reprojection and history validation]], [[Eye movements and display-time gaze]] | [[Foveated rendering and uncertainty]] | Peripheral allocation uses a specified task-dependent angular sensitivity model. |
| [[Optical focus and binocular conflict]], [[Psychophysics and threshold inference]], [[Visual acuity and eccentricity]] | [[Stereoacuity and binocular foveation]] | Optical image formation determines the direction/focus assumptions used by geometry and binocular analysis. |
| [[Psychophysics and threshold inference]], [[Perceptual metrics and display calibration]] | [[Research validity and ablation]] | A perceptual conclusion requires a defined task and inference procedure. |

## Construction and value transfer

| Prerequisites | Dependent | Why the dependency exists |
|---|---|---|
| [[References and aliases]], [[Const qualification and shallow constness]] | [[Parameter passing and sink contracts]] | Parameter access differs according to whether a new object is created or an existing referent is borrowed. |
| [[Value categories and materialization]], [[Object construction and storage reuse]] | [[Return values and copy elision]] | The return expression category distinguishes result construction from a bound path or move candidate. |
| [[Class access and invariants]], [[Object construction and storage reuse]] | [[Constructors and invariant establishment]] | Construction must establish the conditions public operations are allowed to assume. |
| [[Constructors and invariant establishment]] | [[Default and value initialization]] | Initialization form and structural order determine how the constructor establishes members. |
| [[Constructors and invariant establishment]] | [[Member order and delegating constructors]] | Initialization form and structural order determine how the constructor establishes members. |
| [[Member order and delegating constructors]], [[Storage duration and object lifetime]] | [[Destruction and partial construction]] | Reverse cleanup depends on the order and completeness of subobject construction. |
| [[Object identity and generation handles]], [[Ownership and non-owning views]] | [[Copy semantics and resource identity]] | Copying an identifier differs from cloning the resource or application entity. |
| [[Copy semantics and resource identity]], [[Constructors and invariant establishment]], [[Destruction and partial construction]] | [[Assignment and exception guarantees]] | Assignment and moving must implement a defined semantic value rather than a raw pointer copy. |
| [[Move casts and forwarding references]], [[Copy semantics and resource identity]], [[Destruction and partial construction]] | [[Move operations and moved-from states]] | An xvalue cast permits selection of a consuming operation without performing it. |
| [[Copy semantics and resource identity]], [[Move operations and moved-from states]], [[Default and value initialization]] | [[Special member generation and traits]] | Assignment and moving must implement a defined semantic value rather than a raw pointer copy. |
| [[Move operations and moved-from states]], [[Assignment and exception guarantees]], [[Special member generation and traits]] | [[Noexcept and transfer selection]] | Traits, failure policy and relocation must distinguish source survival from source destruction. |
| [[Special member generation and traits]], [[Assignment and exception guarantees]], [[Ownership and non-owning views]] | [[Rules of Three Five and Zero]] | The actual usable construction expression determines trait-controlled transfer selection. |
| [[Move operations and moved-from states]], [[Object construction and storage reuse]] | [[Relocation versus move]] | Traits, failure policy and relocation must distinguish source survival from source destruction. |

## Borrowing, categories and const access

| Prerequisites | Dependent | Why the dependency exists |
|---|---|---|
| [[Storage duration and object lifetime]], [[Declarations and definitions]] | [[References and aliases]] | A reference is an alias established by binding, not a reseatable pointer object. |
| [[Declarations and definitions]] | [[Value categories and materialization]] | lvalues and xvalues identify objects/functions; prvalues produce values/results. |
| [[References and aliases]], [[Value categories and materialization]] | [[Reference binding and converted temporaries]] | Binding chooses an initial referent under type/category rules. |
| [[Reference binding and converted temporaries]], [[Storage duration and object lifetime]] | [[Temporary lifetime extension]] | Ordinary temporary destruction occurs at the full-expression's end unless specified extension applies. |
| [[Value categories and materialization]], [[References and aliases]] | [[Auto decltype and reference collapsing]] | Ordinary by-value auto drops top-level const/reference information; auto& preserves referent qualification. |
| [[Auto decltype and reference collapsing]], [[Value categories and materialization]] | [[Move casts and forwarding references]] | std::move is an xvalue cast preserving constness, not a resource transfer. |
| [[Pointer arithmetic and provenance]] | [[Null pointers and optional access]] | A null pointer designates no object/function; nullptr_t converts to appropriate null pointer values without choosing an integer overload. |
| [[Pointer arithmetic and provenance]], [[Object identity and generation handles]] | [[Borrow invalidation and logical identity]] | A borrow relies on an external lifetime/mutation contract. |
| [[Borrow invalidation and logical identity]] | [[Borrow contracts and pool epochs]] | An epoch versions a borrow-validity interval. |
| [[Ownership and non-owning views]], [[Temporary lifetime extension]], [[Borrow invalidation and logical identity]] | [[Deferred captures and borrowed views]] | A span or string_view describes data without owning it. |
| [[References and aliases]] | [[Const qualification and shallow constness]] | Const restricts modification through an access path; another mutable alias can change a mutable referent. |
| [[Const qualification and shallow constness]] | [[Multilevel pointer qualification]] | int** cannot implicitly become const int**: that would permit injection of an actually const object's address into an original int*. |
| [[Const qualification and shallow constness]], [[Value categories and materialization]], [[Temporary lifetime extension]] | [[Ref-qualified accessors]] | Member &/&& qualifiers constrain the caller's object category. |
| [[Const qualification and shallow constness]], [[Class access and invariants]] | [[Logical constness and mutable caches]] | Logical constness preserves public meaning while eligible mutable data members may change private state. |

## C++ - From buildable text to live objects

The C++ volume accepts Parts 101–147. Construction/value transfer now develop the object-and-borrow foundation without conflating lifetime, identity, category, ownership, failure guarantees or expression traits. Code examples and runtime/build traces live directly in [[Continuous Notes/02 - C++ - From Objects to Reliable Programs|Volume 02]].

| Prerequisite | Dependent | Reason |
|---|---|---|
| [[Compiler IR and machine instructions]] | [[Translation units and linking]] | Compilation and emitted code explain why separately translated units need a link step. |
| [[Translation units and linking]] | [[Declarations and definitions]] | A caller needs declaration/type information before the defining implementation is available. |
| [[Declarations and definitions]] | [[One Definition Rule]] | Agreement about which declarations define entities is necessary to state the one-definition constraint. |
| [[Declarations and definitions]] | [[Linkage and entity ownership]] | Declarations must be related to determine whether they denote one entity or distinct private entities. |
| [[Declarations and definitions]] | [[Scopes and name lookup]] | A declaration binds a name before scope and lookup can determine which binding is found. |
| [[Scopes and name lookup]] | [[Argument-dependent lookup]] | Ordinary lookup provides candidates and suppression rules before arguments can add associated candidates. |
| [[Translation units and linking]] | [[C++ modules and reachability]] | Interface information serves compilation, while object code serves linking; modules change their distribution. |
| [[Translation units and linking]] | [[Preprocessing and macro boundaries]] | The compilation boundary explains where token replacement stops and typed semantic analysis begins. |
| [[Storage duration and object lifetime]] | [[Object construction and storage reuse]] | Backing-storage duration must be distinguished from the occupant's start, destruction and replacement. |
| [[Object construction and storage reuse]] | [[Object identity and generation handles]] | Storage reuse creates successive occupants, motivating handles that record the intended generation. |
| [[Fixed-width integer types]] | [[Scoped enums and validated domains]] | An enum's underlying integer type establishes representation but not the application's valid code set. |
| [[Declarations and definitions]] | [[Aggregate initialization]] | The defined class's members and constructors determine whether aggregate element initialization applies. |
| [[Declarations and definitions]] | [[Class access and invariants]] | Class declarations establish members and bases whose access must be public or controlled. |
| [[ABI and object layout]] | [[Standard-layout and trivial copyability]] | ABI offsets and padding must be separated from standard-layout and byte-copy language eligibility. |
| [[Pointer arithmetic and provenance]] | [[Named members versus array elements]] | Array-relative pointer bounds explain why adjacency of separate members cannot authorize indexing. |
| [[Storage duration and object lifetime]] | [[Thread-local storage]] | Thread duration is a distinct backing-storage category, not a synonym for a local name's scope. |

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

## Narrative order and source coverage

The first six sources cover Parts 1–100. Volume 01 follows physical state → interpretation → decisions/fields → finite arithmetic → floating representation/error → deliberate precision choices → placement/lifetime → serialization/integrity/dump. Source order remains in Foundations. M007 now opens Volume 02 with supplied Parts 101–112; the earlier prediction of 101–116 is not accepted coverage.

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

The first volume covers all Parts 1–100. Prerequisites explain the story's dependencies; they do not require leaving the continuous volume to learn an essential definition.

## M005 - Arithmetic, error and reproducibility learning spine

| Prerequisites | Dependent | Why these steps are needed |
|---|---|---|
| [[Positional notation]], [[Binary32]] | [[Terminating fractions and dyadic rationals]] | A reduced denominator must contain only factors of two to terminate in binary. |
| [[Significand and hidden bit]], [[Exponent bias]], [[Guard round and sticky bits]] | [[Floating-point addition and absorption]] | Align scales before addition; discarded digits influence rounding or disappear. |
| [[Unit roundoff]] | [[Absolute and relative error]] | Separate error magnitude from error relative to a nonzero reference. |
| [[Absolute and relative error]], [[Conditioning and singularity]] | [[Forward and backward error]] | Distinguish output discrepancy from the input perturbation that explains it. |
| [[Unit roundoff]], [[Absolute and relative error]] | [[Accumulated rounding error]] | Bounds depend on rounding count, range assumptions and operand magnitudes. |
| [[Absolute and relative error]], [[Significand and hidden bit]], [[Conditioning and singularity]] | [[Cancellation and loss of significance]] | A small difference exposes input uncertainty relative to its size. |
| [[Cancellation and loss of significance]], [[Subnormals and gradual underflow]] | [[Sterbenz lemma]] | Nearby stored operands can subtract exactly even though their original inputs were rounded. |
| [[Cancellation and loss of significance]] | [[Stable reformulation]] | Change intermediates to avoid unnecessary loss while preserving the mathematical target. |
| [[IEEE-754 floating point]], [[Rounding to nearest ties to even]] | [[Fused multiply-add]] | One final rounding differs from separate product and sum roundings. |
| [[Floating-point addition and absorption]], [[Fused multiply-add]] | [[Floating-point reassociation]] | Expression grouping changes rounded intermediates. |
| [[Floating-point reassociation]], [[Accumulated rounding error]] | [[Compensated and pairwise summation]] | Correction terms and balanced trees address different accumulation mechanisms. |
| [[Compensated and pairwise summation]] | [[Deterministic reduction tree]] | Fix leaves and grouping to make reduction order repeatable. |
| [[Deterministic reduction tree]], [[Floating-point environment and compiler modes]], [[Atomic operations and write contention]] | [[Numerical reproducibility]] | Order, arithmetic environment and synchronization are separate contracts. |
| [[Binary32]], [[Binary64]], [[Rounding to nearest ties to even]] | [[Mixed-precision state updates]] | Wider computation cannot restore information discarded by persistent narrowing. |
| [[Mixed-precision state updates]], [[Arithmetic policies]] | [[Capped weighted updates]] | Capping changes the recurrence and creates exact history dependence before roundoff. |
| [[Absolute and relative error]], [[Directed rounding modes]] | [[Numerical decision boundaries]] | Small numeric changes can cross pixel, sign or threshold decisions. |
| [[Cancellation and loss of significance]], [[Fused multiply-add]], [[Compensated and pairwise summation]] | [[Dot and cross-product rounding]] | Input representation, product rounding and summation are separate sources of error. |

## Representation choices and external contracts

| Prerequisites | Dependent | Why these steps are needed |
|---|---|---|
| [[Two's complement]], [[Interpretation contract]] | [[Fixed-point representation]] | A raw integer I represents I/S for an agreed positive scale S and units. With S=2^16, raw 212992 means 3.25. The integer has no built-in fractional point. |
| [[Fixed-point representation]] | [[Fixed-point range and resolution]] | For signed N-bit raw storage and F fraction bits, step is 2^-F and the positive endpoint is (2^(N-1)-1)/2^F. Required step 0.001 and magnitude 1e6 in 32 bits allow F=10 or 11. |
| [[Fixed-point representation]], [[Minimum signed value]], [[Arithmetic policies]] | [[Fixed-point rescaling]] | Raw multiplication changes scale S to S²; divide the wide product by S with a declared rounding rule. Raw division uses A*S/B. Check the destination after widening, and handle divisor zero and signed minima. |
| [[Fixed-point representation]], [[Absolute and relative error]] | [[Affine quantization and zero point]] | A positive step s, integer zero point z and bounded code q reconstruct x̂=s(q-z). Nearest encode uses clamp(round(x/s)+z). Half-step error assumes no clipping and ideal parameters. |
| [[Affine quantization and zero point]], [[Bit packing]] | [[Groupwise quantization and metadata]] | Local groups share a scale, improving adaptation at a metadata cost. Symmetric INT4 calibration maxAbs/7 reconstructs codes around [-7,7]; float scales add about 32/G bits per value for full groups. |
| [[Quantization and compressed weights]] | [[One-bit centroid quantization]] | One sign label selects one of two row means. Row [-1,-3,2,4] reconstructs [-2,-2,3,3], with RMSE one. Two float32 centroids cost 64 additional bits per row. |
| [[Affine quantization and zero point]] | [[UNORM and SNORM]] | UNORM decodes q/(2^n-1); UNORM8 code128 gives128/255. SNORM commonly clamps q/(2^(n-1)-1) at -1, so -128 and -127 both map to -1 for eight bits. |
| [[IEEE-754 floating point]], [[Rounding to nearest ties to even]] | [[Binary16]] | IEEE binary16 has 1/5/10 fields, bias15 and11 normal precision bits. Word3E00 is1.5; maximum finite65504; normal minimum2^-14 and subnormal minimum2^-24. |
| [[IEEE-754 floating point]], [[Rounding to nearest ties to even]] | [[Bfloat16]] | BF16 has1/8/7 fields, bias127 and8 normal precision bits. It keeps broad exponent range at coarse spacing2^-7 above1. A nearest-even conversion must handle NaNs separately from finite rounding. |
| [[Unsigned modular arithmetic]], [[Interpretation contract]] | [[Offset-binary coding]] | A bias turns signed levels into unsigned codes. For the source-shaped INT4 convention c=q+8, -3 becomes nibble0101, not the two's-complement nibble1101. |
| [[Absolute and relative error]], [[Quantization and compressed weights]] | [[Quantization error metrics]] | For N>0, RMSE is sqrt(sum squared discrepancies/N), MAE is mean absolute discrepancy, and max error is the largest observed magnitude. Promote float operands before subtraction when measuring in double. |
| [[Serialization]], [[Bit casting and representation]] | [[Binary format contracts]] | A schema declares field widths/order, byte order, numeric representation, valid values, lengths, version, padding and identity policy. Magic identifies a candidate format; version chooses a schema, neither authenticates content. |
| [[Binary format contracts]], [[Unsigned modular arithmetic]], [[Short-circuit evaluation]] | [[Checked binary parsing]] | Validate products before size calculation, ranges with offset≤size and length≤size-offset, and practical resource limits before allocation. Mapping/alignment does not itself establish valid typed object access. |
| [[Bitwise XOR]], [[Serialization]] | [[Checksums CRCs and hashes]] | Checksums/digests summarize specified bytes. Additive checksums miss reorderings; CRCs use GF(2) polynomial remainders and declared parameters. Cryptographic hashes need a trusted reference to support integrity claims against substitution. |
| [[Checksums CRCs and hashes]], [[Interpretation contract]] | [[Integrity and authenticity]] | Integrity compares content with a declared reference; authenticity requires a trust mechanism such as a MAC or signature. Replacing both file and untrusted digest defeats a simple hash comparison. |
| [[Endianness]], [[Bit casting and representation]], [[Binary format contracts]] | [[Memory dump interpretation]] | Raw octets gain meaning from offsets, width, encoding and byte order. Bytes00 00 80 3F mean integer1065353216 or binary32 one under different little-endian contracts. A dump does not prove a typed object is live. |
