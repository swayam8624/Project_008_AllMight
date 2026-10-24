# Foundations

## Phase 6 - Parts 501-600

The user supplied the Phase 6 group identified as M031–M036 in nineteen pasted documents, including a supplemental panel analogy. Explicit lesson headings cover 99 distinct parts: 501–541 and 543–600. Part 542 is referenced as the next cross-product definition but has no standalone lesson; its mechanism is explicitly reconstructed from adjacent material in the new volume. The narrative therefore develops all 100 phase topics without falsely claiming 100 standalone source lessons.

Main coverage is now the disjoint ranges 1–160 and 501–600: 260 developed topic entries, of which 259 have supplied standalone numbered lessons and one is identified reconstruction. Parts 161–500 and 601 onward remain pending. Accepted chunk groups are M001–M010 and the user-labeled M031–M036, 16 of 180; no unverified per-chunk subdivision of this phase's parts is imposed. XR01/Modules 1–19 remain separate.

[[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code|Read the continuous Phase 6 story]]. Source part numbers are an audit ledger, not lesson segmentation.

| Part | Supplied topic, expanded mechanism and narrative node |
|---|---|
| 501 | SCALARS AS ELEMENTS OF A NUMBER FIELD — Field laws make nonzero division and vector scaling meaningful; a floating type is only an approximation. [[Scalars fields and real-number models]] |
| 502 | REAL NUMBERS AND THE PROPERTIES GRAPHICS RELIES ON — Order, intervals and completeness justify real geometry; bounded machine values are not the complete number line. [[Scalars fields and real-number models]] |
| 503 | COMPLEX NUMBERS AND TWO-DIMENSIONAL ROTATION INTUITION — Unit complex multiplication rotates a plane; conjugation reverses it, and magnitude drift adds unwanted scale. [[Complex numbers and planar rotations]] |
| 504 | VECTORS AS GEOMETRIC DISPLACEMENTS — Free displacement survives translation of its starting point; location and movement are different roles. [[Vectors coordinates and affine points]] |
| 505 | VECTORS AS COORDINATE TUPLES — An ordered tuple gives components, but neither its basis nor physical role follows from storage shape. [[Vectors coordinates and affine points]] |
| 506 | THE DISTINCTION BETWEEN A VECTOR AND ITS COORDINATES — One physical vector has different coordinates in different bases; distinguish passive description from active motion. [[Vectors coordinates and affine points]] |
| 507 | VECTOR SPACES AND THEIR AXIOMS — Closure and vector-space laws enable consistent combinations; unit directions are a constrained non-example. [[Vector spaces subspaces and span]] |
| 508 | SUBSPACES — A nonempty subset closed under arbitrary two-vector combinations is a subspace; shifted planes need affine language. [[Vector spaces subspaces and span]] |
| 509 | LINEAR COMBINATIONS — Arbitrary coefficients generate movements, not necessarily averages; negative weights are valid. [[Vector spaces subspaces and span]] |
| 510 | SPAN — The span is the smallest subspace containing generators; redundant inputs do not enlarge it. [[Vector spaces subspaces and span]] |
| 511 | LINEAR INDEPENDENCE — A zero combination must have only zero coefficients for independence; near dependence is a numerical question. [[Basis coordinates and dimension]] |
| 512 | BASIS VECTORS — Spanning ensures reachability and independence ensures uniqueness; both are necessary for a basis. [[Basis coordinates and dimension]] |
| 513 | DIMENSION OF A VECTOR SPACE — Dimension counts basis elements over the selected field; a plane stored in three components still has dimension two. [[Basis coordinates and dimension]] |
| 514 | COORDINATE REPRESENTATION RELATIVE TO A BASIS — Coordinates weight an ordered basis; reconstruct the physical arrow before interpreting the tuple. [[Basis coordinates and dimension]] |
| 515 | CHANGING BASIS CONCEPTUALLY — Changing coordinates solves the new basis relation; near-parallel basis axes amplify coefficient uncertainty. [[Basis coordinates and dimension]] |
| 516 | STANDARD BASIS IN $\mathbb R^2$ — The two standard unit axes reconstruct every planar coordinate; visual up/right conventions remain separate. [[Basis coordinates and dimension]] |
| 517 | STANDARD BASIS IN $\mathbb R^3$ — Three standard unit axes reconstruct spatial coordinates; no universal camera forward direction follows. [[Basis coordinates and dimension]] |
| 518 | VECTOR ADDITION FROM THE PARALLELOGRAM RULE — Head-to-tail and parallelogram movement yield componentwise addition in a common basis. [[Vectors coordinates and affine points]] |
| 519 | SCALAR MULTIPLICATION OF VECTORS — Scalar scaling stretches and may reverse direction; velocity times elapsed time yields displacement with correct units. [[Vectors coordinates and affine points]] |
| 520 | VECTOR SUBTRACTION AS DISPLACEMENT DIFFERENCE — Subtraction adds the opposite vector; ordered point difference produces the displacement from one location to another. [[Vectors coordinates and affine points]] |
| 521 | AFFINE POINTS VERSUS FREE VECTORS — Affine points have no intrinsic zero; point differences and translations are valid without choosing an origin. [[Vectors coordinates and affine points]] |
| 522 | POSITION VECTORS AND WHY POINTS ARE NOT MATHEMATICALLY VECTORS — A position vector is point minus chosen origin; changing the origin cancels only in appropriate differences. [[Vectors coordinates and affine points]] |
| 523 | DIRECTION VECTORS — Direction answers which way and must be nonzero; translation does not affect it. [[Vectors coordinates and affine points]] |
| 524 | DISPLACEMENT VECTORS — Displacement records change in position and may validly be zero; units and frame must match. [[Vectors coordinates and affine points]] |
| 525 | UNIT VECTORS — Unit vectors retain direction but remove magnitude; the zero vector cannot satisfy this promise. [[Vectors coordinates and affine points]] |
| 526 | EUCLIDEAN VECTOR MAGNITUDE — Pythagoras yields Euclidean length only in orthonormal coordinates; a slanted basis needs its metric. [[Norms lengths and induced distances]] |
| 527 | SQUARED MAGNITUDE AND WHY GRAPHICS OFTEN AVOIDS SQUARE ROOTS — Squared length avoids some square roots in comparisons but is neither a norm nor a metric. [[Norms lengths and induced distances]] |
| 528 | NORMALIZING A VECTOR — Divide a nonzero vector by its length to normalize; this operation is nonlinear. [[Unit directions and safe normalization]] |
| 529 | ZERO-VECTOR NORMALIZATION AND ROBUST HANDLING — Reject zero/non-finite inputs and scale before squaring; application noise thresholds are a separate policy. [[Unit directions and safe normalization]] |
| 530 | L1 NORM — The first norm sums absolute coordinates; its unit ball is a coordinate-dependent diamond. [[Norms lengths and induced distances]] |
| 531 | L2 NORM — The second norm is Euclidean; Cauchy–Schwarz proves its triangle inequality. [[Norms lengths and induced distances]] |
| 532 | L-INFINITY NORM — The infinity norm is the maximum absolute component; its unit ball is a coordinate-dependent box. [[Norms lengths and induced distances]] |
| 533 | DISTANCE INDUCED BY A NORM — A norm induces a metric by measuring point difference; different norms can choose different nearest points. [[Norms lengths and induced distances]] |
| 534 | DOT PRODUCT ALGEBRAIC DEFINITION — The real-coordinate dot sums component products under an orthonormal metric assumption. [[Dot products and orthogonality]] |
| 535 | DOT PRODUCT GEOMETRIC INTERPRETATION — Dot measures magnitude-weighted alignment; it equals cosine only for unit inputs. [[Dot products and orthogonality]] |
| 536 | DERIVING THE COSINE-ANGLE RELATION FOR THE DOT PRODUCT — Compare an expanded squared difference with the cosine rule to derive the angle relation. [[Dot products and orthogonality]] |
| 537 | ORTHOGONAL VECTORS — Orthogonality is zero inner product; zero has no angle, and numerical tests need scale-aware criteria. [[Dot products and orthogonality]] |
| 538 | VECTOR PROJECTION ONTO ANOTHER VECTOR — Require the along-axis remainder to be orthogonal to derive the target squared-length denominator. [[Projection rejection and scalar components]] |
| 539 | VECTOR REJECTION FROM ANOTHER VECTOR — Rejection removes the projected component and enables tangent-plane movement. [[Projection rejection and scalar components]] |
| 540 | SCALAR PROJECTION — Signed scalar projection is length along a target direction, not the coefficient multiplying the target vector. [[Projection rejection and scalar components]] |
| 541 | GRAM MATRIX CONCEPT — Pairwise basis dots form a Gram metric; coordinates measure physical lengths through this matrix. [[Gram matrices and coordinate metrics]] |
| 542 | Cross product definition in $\mathbb R^3$ — Reconstructed enrichment: perpendicular oriented-area vector definition precedes the supplied determinant derivation. [[Cross products and oriented area]] |
| 543 | CROSS PRODUCT DETERMINANT DERIVATION — Signed coordinate-plane minors yield the cross formula; direct expansion verifies perpendicularity. [[Cross products and oriented area]] |
| 544 | CROSS PRODUCT GEOMETRIC MEANING — Cross produces an area normal, not merely an arbitrary perpendicular direction. [[Cross products and oriented area]] |
| 545 | CROSS-PRODUCT MAGNITUDE AND PARALLELOGRAM AREA — Cross magnitude equals base times height; triangle area is half the parallelogram area. [[Cross products and oriented area]] |
| 546 | CROSS-PRODUCT ORIENTATION AND THE RIGHT-HAND RULE — Input order and a chosen positive orientation fix the cross sign; mirrored transforms add a determinant sign. [[Coordinate handedness and orientation]] |
| 547 | WHY THE CROSS PRODUCT IS SPECIAL TO THREE DIMENSIONS — Ordinary graphics cross identifies bivectors with vectors in three dimensions; higher dimensions need qualifications. [[Coordinate handedness and orientation]] |
| 548 | SCALAR TRIPLE PRODUCT — Dotting an oriented area with a third movement produces a signed scalar volume. [[Triple products and signed volume]] |
| 549 | TRIPLE PRODUCT AS SIGNED VOLUME — Triple product is parallelepiped volume; a four-point tetrahedron has one sixth of its absolute value. [[Triple products and signed volume]] |
| 550 | VECTOR TRIPLE-PRODUCT IDENTITY — Expand a nested cross into dot-weighted vectors; cross grouping is not associative. [[Triple products and signed volume]] |
| 551 | ANGLES BETWEEN VECTORS — Nonzero finite directions define an unsigned angle in the interval from zero to pi. [[Robust vector angles]] |
| 552 | ROBUST ANGLE COMPUTATION WITH `atan2` — Scale first and use atan2 of cross length and dot; a signed angle additionally needs an oriented plane. [[Robust vector angles]] |
| 553 | REFLECTION OF A DIRECTION ACROSS A NORMAL — Reverse the normal component to reflect across a plane; unit-normal and general-normal formulas differ. [[Reflection refraction and shading directions]] |
| 554 | REFRACTION GEOMETRY FROM SNELL'S LAW — Snell scales the tangent component; a negative transmitted squared cosine indicates total internal reflection. [[Reflection refraction and shading directions]] |
| 555 | HALF-VECTOR CONSTRUCTION IN SHADING — Normalize the sum of unit outward light/view directions; opposite inputs have no unique half-vector. [[Reflection refraction and shading directions]] |
| 556 | TANGENT VECTORS ON A SURFACE — Surface derivatives or triangle edges span tangent movement; texture tangents may retain physical stretch. [[Surface tangents normals and bitangents]] |
| 557 | NORMAL VECTORS ON A SURFACE — Geometric normals measure perpendicularity; shading normals can differ and need their own contract. [[Surface tangents normals and bitangents]] |
| 558 | BITANGENT VECTORS — Solve edge/UV equations for a second tangent; degenerate UVs and mirrored-island signs need explicit handling. [[Surface tangents normals and bitangents]] |
| 559 | ORTHONORMAL BASES — Unit perpendicular axes give an identity Gram matrix and simple dot-based reconstruction. [[Orthonormal frames and Gram-Schmidt]] |
| 560 | CONSTRUCTING AN ORTHONORMAL BASIS FROM ONE DIRECTION — A least-aligned helper yields a nondegenerate frame from one direction, but does not ensure continuity. [[Orthonormal frames and Gram-Schmidt]] |
| 561 | GRAM-SCHMIDT ORTHOGONALIZATION — Repeated projection removal preserves span while producing perpendicular axes for independent generators. [[Orthonormal frames and Gram-Schmidt]] |
| 562 | MODIFIED GRAM-SCHMIDT AND NUMERICAL STABILITY — Measuring updated residuals changes rounding behavior; reorthogonalization cannot repair uncertain input rank. [[Orthonormal frames and Gram-Schmidt]] |
| 563 | HANDEDNESS OF COORDINATE BASES — Triple-product sign classifies an ordered basis relative to chosen orientation, not memory layout. [[Coordinate handedness and orientation]] |
| 564 | RIGHT-HANDED COORDINATE SYSTEMS — Positive frame orientation is preserved by proper rotations and cyclic axis permutations. [[Coordinate handedness and orientation]] |
| 565 | LEFT-HANDED COORDINATE SYSTEMS — A reflected frame has opposite orientation while preserving Euclidean distances; API conventions need separate statements. [[Coordinate handedness and orientation]] |
| 566 | ORIENTATION TESTS IN 2D USING SIGNED AREA — Signed doubled triangle area supplies a two-dimensional orientation predicate with a documented axis convention. [[Coordinate handedness and orientation]] |
| 567 | ORIENTATION TESTS IN 3D USING TRIPLE PRODUCTS — Four-point triple products decide plane side and coplanarity under an explicit ordering convention. [[Coordinate handedness and orientation]] |
| 568 | BARYCENTRIC COORDINATES AS AFFINE WEIGHTS — Triangle weights sum to one for affine meaning; nonnegative weights identify the convex interior. [[Barycentric coordinates and interpolation]] |
| 569 | DERIVING BARYCENTRIC COORDINATES FOR A TRIANGLE — Edge Gram solves or signed area ratios recover weights; degeneracy and off-plane residuals remain separate checks. [[Barycentric coordinates and interpolation]] |
| 570 | INTERPOLATING ATTRIBUTES WITH BARYCENTRIC COORDINATES — The same affine weights interpolate attributes; nonlinear encoding and unit length require additional policies. [[Barycentric coordinates and interpolation]] |
| 571 | AFFINE COMBINATIONS VERSUS LINEAR COMBINATIONS — Only sum-one weights preserve point translation; sum-zero weights describe a displacement. [[Convex combinations and convex hulls]] |
| 572 | CONVEX COMBINATIONS — Nonnegative affine weights produce convex mixtures; extrapolation is affine but not convex. [[Convex combinations and convex hulls]] |
| 573 | CONVEX SETS — A convex set contains every line segment between its members; a boundary need not be convex. [[Convex combinations and convex hulls]] |
| 574 | CONVEX HULL INTUITION — A convex hull is all finite convex mixtures and may fill source concavities or holes. [[Convex combinations and convex hulls]] |
| 575 | MINKOWSKI SUM INTUITION — Pairwise set-coordinate addition represents sweeps and expansions; symmetric robot dilation is a special case. [[Minkowski sums and support mappings]] |
| 576 | MINKOWSKI DIFFERENCE INTUITION — Collision difference means A plus reflected B here, not set subtraction; overlap is origin membership. [[Minkowski sums and support mappings]] |
| 577 | SUPPORT FUNCTIONS OF CONVEX SHAPES — Support scalar and support point differ; opposite-direction support queries construct difference extrema on demand. [[Minkowski sums and support mappings]] |
| 578 | HOMOGENEOUS COORDINATES AS PROJECTIVE REPRESENTATIVES — Nonzero common scale defines projective equivalence; finite representatives divide by their final coordinate. [[Homogeneous coordinates and perspective division]] |
| 579 | POINTS AND DIRECTIONS THROUGH HOMOGENEOUS $w$ — Homogeneous one distinguishes points from zero-tagged directions under affine translation; zero tuple is invalid projectively. [[Homogeneous coordinates and perspective division]] |
| 580 | NORMALIZED DEVICE-COORDINATE INTUITION AS A VECTOR-SPACE MAPPING — NDC standardizes device-coordinate ranges, not length; perspective division is nonlinear and follows clipping. [[Homogeneous coordinates and perspective division]] |
| 581 | DUAL VECTORS AND COVECTORS — Covectors are linear scalar measurement rules; a metric hides their distinction from ordinary vectors. [[Covectors linear functionals and normal transforms]] |
| 582 | WHY SURFACE NORMALS TRANSFORM DIFFERENTLY FROM POSITIONS — Preserve the normal's pairing with transformed tangents to derive inverse transpose; singular cases need policy. [[Covectors linear functionals and normal transforms]] |
| 583 | LINEAR FUNCTIONALS — A linear functional distributes across weighted vector sums; its kernel contains movements it measures as zero. [[Covectors linear functionals and normal transforms]] |
| 584 | EXTERIOR-PRODUCT INTUITION — Exterior algebra retains areas and volumes without requiring a vector-valued cross in every dimension. [[Exterior products and geometric algebra]] |
| 585 | WEDGE PRODUCT AS ORIENTED AREA CONCEPT — Antisymmetric bilinear wedges encode oriented area; dependent generators have zero wedge. [[Exterior products and geometric algebra]] |
| 586 | GEOMETRIC-ALGEBRA MOTIVATION FOR GRAPHICS — The geometric product combines dot and wedge; plane elements connect to complex-like rotation structure. [[Exterior products and geometric algebra]] |
| 587 | FLOATING-POINT VECTOR STORAGE IN C++ — Floating storage has bounded precision and target layout; trivial copying does not certify external representation. [[Vector storage SIMD and boundary layouts]] |
| 588 | VEC2 DATA STRUCTURE DESIGN — A two-component value type needs initialization, safe access and explicit semantic roles. [[Vector value types and component access]] |
| 589 | VEC3 DATA STRUCTURE DESIGN — Three-component storage supports spatial operations but is not automatically a point, normal or unit direction. [[Vector value types and component access]] |
| 590 | VEC4 DATA STRUCTURE DESIGN — Four-component storage can serve distinct roles; homogeneous coordinates and quaternions require separate contracts. [[Vector value types and component access]] |
| 591 | OPERATOR OVERLOADS FOR VECTOR ARITHMETIC — Overloads should express unambiguous vector arithmetic; name dot, cross and componentwise multiplication. [[Vector value types and component access]] |
| 592 | SWIZZLES AND COMPONENT ACCESS — Read swizzles reorder components; writable swizzles need temporary-copy and duplicate-destination policies. [[Vector value types and component access]] |
| 593 | ALIGNMENT OF VECTOR TYPES — Measure alignment, size and stride; greater alignment may add padding and is not a performance guarantee. [[Vector storage SIMD and boundary layouts]] |
| 594 | PACKED VERSUS ALIGNED VEC3 REPRESENTATIONS — Compact storage and aligned compute types can coexist with explicit conversion at boundaries. [[Vector storage SIMD and boundary layouts]] |
| 595 | SIMD IMPLEMENTATION OF VECTOR ARITHMETIC — SIMD within one vector differs from SIMD across objects; layouts, reductions and FMA affect throughput and results. [[Vector storage SIMD and boundary layouts]] |
| 596 | APPROXIMATE VECTOR EQUALITY — Exact, componentwise, distance, angular and ULP comparisons answer different questions; closeness is not transitive. [[Vector tolerances and finite-state diagnostics]] |
| 597 | DEBUGGING NaN VECTORS — Catch the first invalid input or operation using finite checks; NaN propagation can silently skip decisions. [[Vector tolerances and finite-state diagnostics]] |
| 598 | VECTOR INVARIANTS IN ENGINE APIs — Validated wrappers establish finite/unit promises and operations must preserve or explicitly drop them. [[Coordinate-space types and validated invariants]] |
| 599 | DISTINGUISHING WORLD-SPACE AND LOCAL-SPACE VECTOR TYPES — Space-tagged points, vectors and transforms reject broad frame mistakes; dynamic local-frame identity remains a separate issue. [[Coordinate-space types and validated invariants]] |
| 600 | Reconstructing a Production-Ready `Vec3` Library from First Principles — Reconstruct the scalar library from the mathematics, then verify contracts with edge cases and seeded properties. [[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code#A complete scalar laboratory makes the story executable|Inline capstone and tests]] |

## XR01 - Independent seeing and representation source

This source is outside the 3,000-part curriculum. Module labels below preserve the attachment's coverage without assigning it main part numbers. XR01 is one accepted source in a separate anticipated 3–4-source continuous series; future inputs are pending. Main coverage is recorded separately above: Parts 1–160 and 501–600, with the Part 542 source gap identified.

| Independent source module | Mechanism and teaching location |
|---|---|
| XR01/1 - Light | [[Light spectra and radiometry]] and [[Reflectance and rendering equation]]: spectra/receptor response, units, BRDF and transport assumptions. |
| XR01/2 - Eye | [[Retinal encoding and receptive fields]]: anatomy, receptors, phototransduction, center-surround and cortical processing. |
| XR01/3 - Acuity | [[Visual angle and calibrated rays]] and [[Visual acuity and eccentricity]]: angular units, local/average PPD, eccentricity, MAR conventions. |
| XR01/4 - Optics | [[Optical focus and binocular conflict]] and [[PSF OTF and MTF]]: refraction, focus, diffraction, distortion and contrast transfer. |
| XR01/5 - Perception | [[Contrast sensitivity adaptation and masking]], [[Eye movements and display-time gaze]], [[Psychophysics and threshold inference]]: task, timing and detection models. |
| XR01/6 - Signals | [[Continuous signals and discrete samples]]: domains, timestamps, sampling versus quantization and error assumptions. |
| XR01/7 - Fourier | [[Fourier analysis and spectral phase]]: basis projection, DFT/FFT, phase, bins, leakage and amplitude conventions. |
| XR01/8 - Sampling | [[Sampling aliasing and reconstruction]] and [[Spatiotemporal sampling and retinal motion]]: spectral replicas, sinc, prefiltering, moving-pattern frequency. |
| XR01/9 - Filtering | [[Convolution and filter footprints]] and [[Multiscale texture filtering]]: kernels, separability, reconstruction, ringing, footprints and pyramids. |
| XR01/10 - Antialiasing | [[Antialiasing and sample allocation]]: pixel integration, coverage versus shading and SSAA/MSAA distinctions. |
| XR01/11 - Temporal | [[Temporal reprojection and history validation]]: weights, motion/jitter conventions, disocclusion and changing-quality history. |
| XR01/12 - Color | [[Color encoding and linear-light operations]]: transfer/primaries, XYZ, relative luminance, HDR and distinct noise mechanisms. |
| XR01/13 - Quality | [[Perceptual metrics and display calibration]]: reference, MSE/PSNR/SSIM, gaze weighting and model limits. |
| XR01/14 - Papers | [[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Read research as a chain of questions, methods and bounded conclusions|Bounded paper-reading anchors]]; coefficients/study details not fully inspected remain source-reported. |
| XR01/15 - Experiment | [[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Complete numerical and image-space laboratory|Tested proxy experiment]], explicitly not saved GPU work or a human study. |
| XR01/16 - Use cases | [[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Diagnose a symptom by the quantity that failed|Failure-to-measurement table]]. |
| XR01/17 - Further vision | [[Stereoacuity and binocular foveation]], retinal velocity, flicker/task dependence and bias/variance distinctions. |
| XR01/18 - Vocabulary | Definitions appear at first use; filled Concepts and [[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Teach the distinctions from a blank board|Recall desk]] preserve operational distinctions. |
| XR01/19 - Understanding checks | Alias/phase, nonlinear filtering, additive-cost and evidence-reconstruction examples; [[Research validity and ablation]]. |

Source numbering is a coverage ledger, not the narrative's teaching order. The copied “START CHUNK2” footer is not a supplied second source or an instruction to invent it.

## M007 - Parts 101-112

The supplied source has exactly twelve parts, 101–112. The revised M007 was integrated into the same coverage, not counted as new parts. M008 supplies 113–126, M009 supplies 127–147, and M010 supplies 148–160. Volume 02 is 4/12 accepted sources, not completed Parts 101–300.

| Part | Definition, mechanism, and concrete reconstruction cue |
|---|---|
| 101 | [[Translation units and linking]] — Source → preprocessing → semantic compilation → object/BMI → link. A declaration can permit compilation while an absent implementation fails linking; BMI is not object code. |
| 102 | [[Declarations and definitions]] — Every definition is a declaration, not conversely. extern int x declares; int x=0 defines. Forward class is incomplete; fixed-underlying opaque enum is complete. |
| 103 | [[One Definition Rule]] — Multiple translation units must describe coherent entities. Header guards are per-TU; ordinary external definitions and inline/template exceptions have different constraints. |
| 104 | [[Scopes and name lookup]] — Scope belongs to a name; qualified lookup selects a context; hiding changes lookup without deleting the hidden object. ADL adds associated candidates. |
| 105 | [[Storage duration and object lifetime]] — Automatic/static/thread/dynamic classify storage. A block-scope static outlives the block; a local pointer and its dynamic pointee have distinct durations. |
| 106 | [[Object construction and storage reuse]] — Sufficient aligned storage and completed initialization establish an ordinary object lifetime. destroy_at can end the object while the byte buffer remains; casts do not construct arbitrary objects. |
| 107 | [[Object identity and generation handles]] — Equal values need not be one object, and reused addresses host different occupants. A slot/generation needs live-registry validation, not merely a non-sentinel index. |
| 108 | [[Fixed-width integer types]] — sizeof(char)=1 C++ byte; CHAR_BIT≥8. int has minimum capacity, not a universal32-bit width. Exact/least/fast and size_t express different requirements. |
| 109 | [[Scoped enums and validated domains]] — Scoped enumerators and no implicit integer conversion reduce accidents. Explicit underlying width does not validate received codes; states and combinable flags need separate designs. |
| 110 | [[Aggregate initialization]] — C++20/23 aggregate criteria are not the older user-provided-constructor rule. Designators follow member declaration order. Braces may select initializer-list or ordinary constructors instead. |
| 111 | [[Class access and invariants]] — struct defaults members and bases public; class defaults them private. Both are class types. Record/invariant intent is convention, not a syntactic capability limit. |
| 112 | [[Standard-layout and trivial copyability]] — Alignment creates internal/tail padding. Traits do not imply array traversal, packing, serialization or GPU compatibility. Named x/y/z require explicit indexing or actual array storage. |

Read [[Continuous Notes/02 - C++ - From Objects to Reliable Programs|the C++ narrative]] from buildable text through entities, storage, occupants, domains and layout. Source numbers belong in this ledger, not in teaching chapters. Independent code and essential traces remain inline in that volume.

This cumulative supplement retains the per-part definition and coverage ledger. The connected teaching volumes now explain the essential foundations, unfamiliar terms and worked steps directly; readers should not need this ledger to reconstruct a lesson. Concept links provide optional navigation. Examples and clarifications are explanatory material; repository claims retain their evidence status in [[Supplementary/Sources and Code Anchors]].

## M008 - Parts 113-126

All fourteen supplied parts are accepted. Source numbering audits coverage; it does not divide the teaching story. M009 now develops parameter passing and value transfer in Parts127–147.

| Part | Definition, mechanism, and concrete reconstruction cue |
|---|---|
| 113 | [[References and aliases]] — Binding establishes an alias; assignment acts on the referent. A reference-to-pointer can reseat that pointer, unlike a reference to its pointee. |
| 114 | [[Reference binding and converted temporaries]] — Type/category select the referent. const int& can track the original; const double& can bind a converted snapshot. Arrays keep extent through array references; bit-fields require special handling. |
| 115 | [[Temporary lifetime extension]] and [[Move casts and forwarding references]] — Direct local binding can extend a temporary, not transitive function-return aliases. Braces/parentheses/new exceptions matter; move is a cast, and deduced T&& forwarding collapses references. |
| 116 | [[Value categories and materialization]] — lvalue/xvalue identify, prvalue produces; glvalue and rvalue overlap at xvalue. A named T&& is an lvalue in ordinary use. decltype(name) differs from decltype((name)). |
| 117 | [[Pointer arithmetic and provenance]] — A pointer is an access path, not ownership. Null/object/one-past/invalid states differ; dereference requires live, aligned, type-accessible in-range storage. |
| 118 | [[Null pointers and optional access]] — nullptr distinguishes pointer absence from integer zero. Typed null deletion is valid; bare delete nullptr is ill-formed. Non-null does not prove lifetime. |
| 119 | [[Named members versus array elements]] — Array-relative positions zero through N may be formed; only zero through N-1 dereferenced. Same-array subtraction must fit ptrdiff_t. Adjacent members do not authorize array indexing. |
| 120 | [[Pointer arithmetic and provenance]] — A pointer's storage relationship is not manufactured by a plausible integer. Specified round trips do not resurrect occupants or authorize arbitrary arithmetic; P3407 is discussion, not C++23 law. |
| 121 | [[Borrow invalidation and logical identity]] — Vector reallocation can dangle a borrow; swap assignment can leave a live slot with a different logical owner. Valid handles do not certify previous component references. |
| 122 | [[Deferred captures and borrowed views]] — Reference members, spans, string views and captured this pointers require referent lifetime. Copying a view does not own its buffer; an owning capture supplies a different contract. |
| 123 | [[Const qualification and shallow constness]] and [[Auto decltype and reference collapsing]] — Read-only access is not global immutability. Top-level const is often dropped by value deduction; actual const objects cannot be made writable by casts. |
| 124 | [[Multilevel pointer qualification]] — Separate pointer and pointee qualification. int** to const int** would enable unsafe injection; intermediate const blocks it. Const span descriptors do not imply const elements. |
| 125 | [[Ref-qualified accessors]] — const member functions restrict the object path; &/&& constrain caller category. Persistent getters can borrow; both mutable and const temporary getters need value/deleted policies. |
| 126 | [[Logical constness and mutable caches]] — Eligible mutable members permit representation changes under const access. Cache/source invariants require invalidation and synchronization; return protected snapshots. Mutable closures are not automatically thread-safe. |

### Borrowing source-to-story navigation

| Mechanism | Teaching position |
|---|---|
| Alias, type/category, conversion and deduction | [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Giving another name to a live object|Binding story]] |
| Temporary lifetime, moves and forwarding | [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A temporary needs an explicit lifetime story|Lifetime story]] |
| Pointer validity, container mutation and pool identity | [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#When a valid borrow stops naming the intended thing|Borrow story]] |
| Const paths, getters, cache invariants | [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Read-only is a path, not an ownership or synchronization promise|Access story]] |

## M009 - Parts 127-147

The supplied source contains exactly twenty-one parts. At acceptance, it extended Volume02 to Parts101–147; M010 subsequently added Parts148–160. Teaching headings remain concept-led; exact coverage is recorded in the current ledger.

| Part | Definition, mechanism, and reconstruction cue |
|---|---|
| 127 | [[Parameter passing and sink contracts]] — Argument expressions initialize separate value-parameter objects. Lvalue/xvalue/prvalue routes differ; a sink retains/transforms a value, and copied pointer members still alias. |
| 128 | [[References and aliases]] — Mutable/const/rvalue reference parameters borrow with different access and consumption contracts. Aliased source/destination references can change a sequential update's result. |
| 129 | [[Parameter passing and sink contracts]] — Pointer parameters copy pointer values; T*& or pointer-output protocols can reseat caller state. Null optionality must be documented; array parameters adjust to pointers without extent enforcement. |
| 130 | [[Return values and copy elision]] — Result objects can be constructed directly; NRVO is optional while appropriate same-type prvalue initialization requires no intermediate transfer. Eligible local/parameter return expressions implicitly move in C++23. |
| 131 | [[Ref-qualified accessors]] and [[Temporary lifetime extension]] — Reference/view results retain owner lifetime/invalidation obligations. Temporary getters require policies for const rvalues too; decltype(auto) can return a reference through parentheses. |
| 132 | [[Constructors and invariant establishment]] — Members/bases initialize before the body. Initializers establish state; body assignments replace it. Constructor failure and factory error results are different protocols. |
| 133 | [[Default and value initialization]] — Default, zero, aggregate and value initialization differ. First-declaration defaulting is not an empty user-provided body; braces do not universally initialize every scalar. |
| 134 | [[Member order and delegating constructors]] — Declaration order governs members regardless of colon-list spelling. Provider-before-dependent order establishes correct construction/reverse cleanup. |
| 135 | [[Member order and delegating constructors]] — Delegation centralizes a target initialization path, then runs its own body. The target is the sole initializer; cycles are ill-formed. |
| 136 | [[Destruction and partial construction]] — Destructor body precedes reverse subobject cleanup; failed ordinary construction destroys completed subobjects. Delegating-target completion changes the whole-object cleanup case. |
| 137 | [[Copy semantics and resource identity]] — New-object copying follows members' semantic operations; equal-sign initialization is not assignment. Pointer/reference copies preserve associations rather than clone referents. |
| 138 | [[Assignment and exception guarantees]] — Existing-object replacement must handle old ownership, self-aliasing and failure. Prepare before commit; reference/const members can delete generated assignment. |
| 139 | [[Copy semantics and resource identity]] — Deep/shallow are contract choices, not good/bad labels. Pixel values may duplicate, handles identify, graph clones remap; copy-on-write adds mutation/lifetime/synchronization costs. |
| 140 | [[Move operations and moved-from states]] — New-object moves can take resources and repair source state; scalar/inline members need not become cheaper. Ordinary constructors alone do not suppress implicit move generation. |
| 141 | [[Move operations and moved-from states]] — Move assignment also handles the destination's former resource and self-move. Allocator policy can prevent constant-time adoption. |
| 142 | [[Move casts and forwarding references]] — move changes category, not ownership itself; const remains const. Explicit local-return moves can inhibit NRVO; move_if_noexcept selects a transfer access path. |
| 143 | [[Move operations and moved-from states]] — Source lifetime continues after moving. Valid-but-unspecified is not destroyed or undefined; operation preconditions still apply. A custom empty source needs an explicit maintained invariant. |
| 144 | [[Return values and copy elision]] — Immovable types can use guaranteed direct result construction; a named-local fallback still needs usable operations. Accessible destruction remains required. |
| 145 | [[Rules of Three Five and Zero]] — Destructor/copy constructor/copy assignment are coordinated raw-owner decisions; copying may instead be deleted. It is a design heuristic, not a compiler mandate. |
| 146 | [[Special member generation and traits]] — Five adds both moves. User-declared versus user-provided matters; a defaulted destructor suppresses dedicated implicit moves, while move-constructible traits can still classify copying as viable. |
| 147 | [[Rules of Three Five and Zero]] — Compose tested member owners rather than repeat transfer/release logic. Zero does not forbid ordinary constructors or prove cross-member invariants; [[Relocation versus move]] remains a versioned research bridge. |

## M010 - Parts 148-160

M010 continues the same Volume 02 story from Rule-of-Zero composition into the concrete ownership and lifetime protocols underneath it. The source specifies Parts 148–160. Repository paths named in the pasted lesson are source-provided examples and remain **To verify** until their repositories/revisions are inspected. These entries identify coverage; the expanded teaching narrative is in [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#From a live object to responsible resource ownership|Volume 02]].

| Part | Definition, mechanism, and concrete reconstruction cue |
|---|---|
| 148 | [[RAII and resource handles]] — Bind matched acquisition/release responsibility to a live owner; scope exit handles early return and unwinding. External asynchronous use can require deferred reclamation. |
| 149 | [[RAII and resource handles]] — Each fully acquired resource must immediately enter an owner; completed subobjects clean up if construction fails. CPU ownership does not alone establish GPU completion. |
| 150 | [[Ownership and non-owning views]] — Ownership means responsibility for ending use/lifetime; a borrow provides access without extending lifetime. |
| 151 | [[Exclusive and shared smart-pointer ownership]] — `unique_ptr` is a move-only exclusive owner; `get()` borrows while `release()` transfers responsibility. |
| 152 | [[Exclusive and shared smart-pointer ownership]] — A custom deleter pairs a native handle with its actual release protocol; deleter state/type can affect representation and conversion safety. |
| 153 | [[Exclusive and shared smart-pointer ownership]] — `shared_ptr` copies join a control block's strong ownership group; object and control-block lifetimes may differ. |
| 154 | [[Exclusive and shared smart-pointer ownership]] — Reference-count operations have costs; control-block coordination does not synchronize arbitrary pointee operations. |
| 155 | [[Weak observation and ownership cycles]] — A weak edge avoids extending object lifetime; `lock()` safely attempts to obtain a temporary strong owner and can break cycles. |
| 156 | [[Weak observation and ownership cycles]] — `enable_shared_from_this` joins an existing owner group; never build an independent `shared_ptr` control block from `this`. |
| 157 | [[Placement construction and explicit lifetime management]] — Allocation functions acquire raw storage; new-expressions combine allocation and initialization. Size, alignment, construction, failure cleanup and deallocation must match. |
| 158 | [[Placement construction and explicit lifetime management]] — Arrays require correct count-aware construction/destruction and matching `new[]`/`delete[]`; containers usually own this protocol more safely. |
| 159 | [[Placement construction and explicit lifetime management]] — Placement construction/`construct_at` begins an object lifetime in adequate pre-existing storage; it does not allocate that storage. |
| 160 | [[Placement construction and explicit lifetime management]] — Explicit destruction ends an occupant's lifetime, not necessarily storage. Reuse, transparent replacement and laundering have distinct conditions. |

The source discusses resource owners, smart pointers, manual allocation, placement construction, array ownership and lifetime restart. Its repository examples (`KairoRenderer/VulkanBuffer.cppm`, `Maveb/.../MetalPtr.hpp`) are **not** represented as live-inspected code findings here.

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#From a live object to responsible resource ownership|Continuous teaching narrative]] · [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall|Active-recall visual board]] · [[Keyword Index]] · [[Dependency Map]]

### Value-transfer source-to-story navigation

| Teaching question | Narrative location |
|---|---|
| What crosses a call, and what can the callee retain? | [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A value crosses the function boundary|Parameter and sink contracts]] |
| How can a result become an owning object without a hidden copy? | [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Return a result without handing out a lifetime problem|Result construction]] |
| How does initial state become valid, then get cleaned up? | [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Construction establishes a usable state|Construction-to-cleanup story]] |
| What does copying or moving this value mean? | [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Copy a meaning rather than just its bytes|Copy, assignment and transfer story]] |
| Which boilerplate should exist at all? | [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#From three repairs to five decisions to zero boilerplate|Composition and ownership]] |

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
