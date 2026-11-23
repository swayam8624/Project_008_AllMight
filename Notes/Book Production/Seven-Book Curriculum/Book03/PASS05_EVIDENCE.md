# Book 03 — Pass 05 authored evidence, exactness contract and acceptance gates

**Series:** From Bits to Research  
**Book:** 03 — From Geometry to Images  
**Chapter:** 05 — The Triangle That Almost Wasn't There  
**Branch:** curriculum/book03-geometry-images-pass01  
**Date:** 23 November 2026  
**Repository scope:** Notes/Book Production/Seven-Book Curriculum/Book03/ and Code Parts/tools/labs/book03_cpp23/; dedicated .github/workflows Book03 pipeline only  
**Publication output:** intentionally not generated or promoted

## What has actually been written

- The full Chapter 05 reader draft (roughly 5,500 words) begins with the house's roof being wrongly declared collinear, not an unmotivated numerical-analysis lecture. It derives orientation determinant, cancellation, binary64 representation, exact signed-dyadic algorithm, algorithmic limb-size proof, Shewchuk-style *conceptual* error-bound filtering, exact versus approximate containment, scale-sensitive modeling tolerances, and the difference between exact arithmetic and uncertain measurements. It includes many worked examples and reconstruct/falsify/transfer/implement/explain tasks with an answer ledger, plus a natural bridge to 3D geometry in Chapter 06.
- New original SVG figures \`Assets/10-the-vanishing-triangle.svg\`, \`11-subnormal-area.svg\`, \`12-certified-sign-interval.svg\` are placed next to their explanations with accessible title/description. The illustrations explicitly say when a triangle is not drawn to scale, and the interval figure explicitly does NOT claim a particular certified filter is implemented.
- \`include/book03/exact_orient2d.hpp\`: C++23-only, dependency-free exact orientation for six finite represented IEEE-754 binary64 values, with a bounded-integer route and a separate arbitrary-limb signed integer route, plus exact represented-input triangle containment. A pedagogical *naive* diagnostic deliberately rounds its two products separately so the cancellation demonstration is not optimized into a fused computation by an implementation.
- \`tests/exact_orient2d_tests.cpp\`: hard deterministic signed determinant -4 fixture, naive rounded determinant 0 witness, tiny subnormal area, largest finite coordinate cases, input-rounding limitation, zero/invalid/edge cases and **5,000** deterministically generated exact half-integer cross-checks against the separately implemented P03 bounded-integer orientation. Test assertions are committed, not reported as executed.
- \`src/precision_studio.cpp\`: \`book03_precision inspect|point|csv|svg|repl\`; stored inputs printed at binary64 round-trip precision; reports separately display naive determinant, exact sign, algorithm path and explanatory status. CSV and SVG visuals are calculated from the same exact source, with no invented performance data.
- \`Code Parts/tools/labs/book03_cpp23/CMakeLists.txt\`: registers P05 \`book03_precision\`, \`book03_exact_tests\` and \`book03_binary64_exact_orientation_contracts\` alongside prior four lab targets and CTests.
- Dedicated Book 03 GitHub Actions source: compiles all source, runs the five test binaries through CTest, checks exact sign versus naive cancellation, subnormal/overflow fixtures, six-row CSV and generated SVG structure and exercises a REPL query. The workflow's path filters were also expanded to trigger manuscript/figure changes through the editorial source gate.
- \`editorial_gate.py\`: expanded to require Chapter 05, its rigorous predicate vocabulary, code files, all **twelve** original vector figures with actual title/description and the established source directory contracts.
- \`PASS05_LEGACY_AUDIT.md\`: direct inspection of source spans about predicates **and** later camera-frustum/reversed-Z material in preexisting Computing Foundations/34; the user’s existing deep knowledge base was not shortened, moved or deleted.

## Exactness theorem and required assumptions

Every finite IEEE-754 binary64 \(x\) is \(N 2^{-1074}\) for a signed integer \(N\). The encoded exponent and significand are decoded from the supplied 64-bit representation using \`std::bit_cast\`. The exact determinant of six such inputs is an exact signed **integer** multiplied by the positive common power \(2^{-2148}\). Signed big integers built from 32-bit unsigned limbs compute that integer without overflow of 64-bit carry accumulators.

For each multiplication inner loop, \((2^{32}-1)^2+(2^{32}-1)+(2^{32}-1)=2^{64}-1\) fits in \`std::uint64_t\`. The dynamic limb vector holds whatever integer width is required by the finite binary64 coordinate domain. The sign is therefore exact **if** the C++ implementation honors the documented arithmetic and decoding semantics.

The source uses C++ compile-time checks for an eight-byte IEC 559 binary64 \`double\`, 53-bit significand and 32/64-bit unsigned limb widths. It rejects nonfinite coordinates. The fast route reuses the earlier proven exact-int64 domain \([-10^6,+10^6]\) for integer-valued coordinates; all other finite binary64 coordinates go through exact dyadic integer arithmetic, even if their naive determinant happens to be far from zero.

This is not a performance-equivalent reimplementation of Shewchuk's adaptive floating error bound and expansion arithmetic. It does not prove the physical input was measured exactly; nor does exact orientation confer an error-free geometric construction. It does not implement full 3D orientation, in-circle tests, clipping ownership or Delaunay algorithms.

## Mathematical evidence cases

| Fixture | Finite represented input facts | Naive two-rounded-product result | Exact signed result |
|---|---|---|---|
| Catastrophic cancellation | \(A=(0,0), B=(N,N+2), C=(N+2,N+4)\), \(N=10^{16}\), all coordinates representable | 0 (false collinear) | \(-1\) (determinant exactly \(-4\)) |
| Subnormal underflow | \(\eta=2^{-1074}\), \(B=(\eta,0),C=(0,\eta)\) | 0 (positive product underflow) | \(+1\) |
| Extreme product overflow | B=(max finite,0), C=(0,max finite) | infinity (magnitude invalid for ordinary finite double) | \(+1\) |
| Input rounding | \(N+1\) and \(N\) round to same binary64 value | 0 | 0; earlier lost information cannot be repaired |
| Small exact integer | B=(6,0), C=(0,4) | 24 | \(+1\), bounded-int64 path |

These are **analytically derived expectations** and authored test fixtures. They should not be described as measured pass results until the binaries have actually run.

## Source custody and publication state

The new chapter is authored under the established \`Notes/Book Production/Seven-Book Curriculum/Book03/Chapters/\` route. SVG figures live beside the existing nine figures in \`Assets/\`; the C++ lab stays in \`Code Parts/tools/labs/book03_cpp23/\`. The original \`Notes/Continuous Notes/\`, \`Notes/Chapter Edition/\`, \`Notes/Book Production/AllMight Codex/LaTeX/\`, Books 1–2, local \`From Bits to Research/EPUB/\` and \`PDF/\` and publication manifests were not edited.

A branch comparison with \`main\` must be executed at the end of this pass to verify that every changed tracked path remains an addition inside the approved new Book03/workflow folders.

## Validation state

- **Authored:** C++23 source; mathematically reasoned proof; deterministic test cases; CLI/CSV/SVG report source; CMake/CTest registration; GitHub Actions pipeline; original diagrams; preserved legacy source audit.
- **Not independently observed yet:** successful target C++23 compilation, CTest passes, CSV/SVG binary generation, cross-toolchain parity, native Apple silicon behavior, GitHub Actions success, screenshot-level figure QA, end-to-end source compilation or EPUB/PDF publication validation.
- **Not attempted:** Mac whole-book LuaLaTeX build, complete EPUB generation, KDP preview or release. The user expressly intends to do the final book build after the manuscript and labs are developed.

## Next round: Chapter 06 — Step Out of the Page

Take the established 2D point/vector/triangle/affine story into three-dimensional geometry, without jumping prematurely to the camera. Develop 3D points and vectors, dot/cross products, planes, normals, orientation and line/plane intersections; include exact/robust boundaries where feasible, uncertainty for nearly parallel constructions, and a physically understandable 3D house and ball. Write a genuinely cumulative standard-C++23 3D geometry project that can generate its own viewable original figures and tests, and route the original 3D source at subsection level. Maintain all existing paths and leave PDF/EPUB publication to final author review.
