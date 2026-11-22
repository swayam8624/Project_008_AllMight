# Book Two — Pass 07: Exact finite-binary64 orientation with independent rational verification

**Active book:** From Equations to Geometry (Book Two of Seven)
**Development branch:** \`curriculum/book02-mathematics-refinement-pass01\`
**Date:** 20 November 2026

## Result and why it is substantively new

Pass 07 replaces the gap between the C++23 bounded-grid exact-sign routine and the approximate binary64 filter with an **independent standards-only C++23 exact-sign oracle for every finite IEEE-754 binary64 coordinate**. It does not overwrite the earlier routines or the original manuscript. It adds mathematically derived teaching adjacent to the existing robust-predicates research section and connects that capability to the Numerical Geometry Atlas capstone.

The \`src/exact_predicates.hpp\` implementation recovers the sign/mantissa/exponent of each stored coordinate using \`std::bit_cast<uint64_t>\`. It represents a finite double exactly as a signed dyadic rational, aligns exponents using base-2^32 limb shifts, subtracts signed integers, multiplies unsigned arbitrary-length integers, and computes the determinant's true sign without a single rounded floating-point intermediate.

The implementation explicitly requires eight-byte IEEE-754 binary64 \`double\` and rejects NaNs/infinities. It is designed for mathematical inspectability rather than adaptive-expansion performance. Its guarantee concerns the **original stored binary64 input values**, never hypothetical unrounded decimal measurements.

## Verification

- Standalone GCC and Clang C++23 compiler checks passed, including positive, negative, exact-zero, extreme-overflow, smallest-subnormal, nearest-representable-perturbation, and nonfinite-input rejection fixtures.
- A separate temporary C++23 export of 500 random finite IEEE-754 bit-pattern triples matched an independently implemented Python \`Fraction.from_float\` exact-sign oracle on every sample.
- The committed native \`orientation-exact\` and \`orientation-audit\` commands export six boundary cases and 500 further seeded triples. The audit CSV stores every original floating coordinate in lossless hexadecimal notation, plus the exact and approximate-filter signs.
- \`predicate_oracle.py\` checks all **506 native outputs** using exact rational arithmetic, asserting the six boundary signs and checking signed orientation behavior.
- GitHub Actions C++23 CMake/CTest passed **11/11 tests** at commit \`7d5ea00cb29a7248e60a33bad6def28b98c1f57d\`, including the 506-row independent oracle and the eight-checkpoint editorial-source gate. Action: https://github.com/swayam8624/Project_008_AllMight/actions/runs/44690032624
- Follow-up commits extend Book Two's two additive TeX sources and project README/editorial safeguards. Their final workflow results must be read separately; passing source tests is not a complete print proof.

## Manuscript placement

\`Assets/Book02/21-robust-predicates-research.tex\` now teaches:
the IEEE-754 significand/exponent representation; exact dyadic subtraction; integer-limb determinant arithmetic; adversarial near-collinear, subnormal and overflow cases; native CLI and independent oracle execution; and why certified input-bit orientation differs from reconstructing true physical/decimal measurements.

\`Assets/Book02/35-cumulative-numerical-geometry-atlas.tex\` adds the native exact-predicate milestone as an **extension** rather than silently claiming that the rest of the Python research atlas is now C++23.

The original 16 chapter sources remain under the existing Git-blob lock; there are still exactly 36 additive Book Two supplemental TeX sources. No Book One or unrelated-series code has been changed. No full physical PDF or author-cache EPUB was compiled as part of this pass.

## Reproduction on the author's Mac

From the repository root:

\`\`\`bash
git switch curriculum/book02-mathematics-refinement-pass01
git pull --ff-only origin curriculum/book02-mathematics-refinement-pass01
cmake -S tools/labs/book02_cpp23 -B build/book02-cpp23 -DCMAKE_BUILD_TYPE=Release
cmake --build build/book02-cpp23 --parallel 4
ctest --test-dir build/book02-cpp23 --output-on-failure
build/book02-cpp23/book02_workbench orientation-exact
build/book02-cpp23/book02_workbench orientation-audit > build/book02-cpp23/orientation-audit.csv
python3 tools/labs/book02_cpp23/predicate_oracle.py build/book02-cpp23/book02_workbench
python3 tools/labs/book02_cpp23/editorial_gate.py
\`\`\`

The test suite uses Python3 only for independent checking, never for the actual algorithm. Its output rows are produced entirely by C++23.

## Boundaries and remaining work

The older \`orient_filtered\` function has a heuristic floating error bound, not a mathematical certificate for all binary64 edge cases. The new full exact-sign routine, not that filter, is authoritative for finite values. The exact integer implementation is relatively slow, has no formal runtime benchmarks or adaptive path, and should not be described as a reimplementation of Shewchuk's 1997 expansion algorithm.

The next native work: sparse triangle stiffness and mass matrices, finite-element heat diffusion and an anchored Poisson/geodesic client; documented row/triangle units and boundary invariants; true visualization generated from C++ outputs; independent multiresolution numerical references; and later 3D predicates/incircle and runtime/performance analysis.

Full production Book Two print typography, figure layout, reflowable EPUB, font embedding, all original-chapter claims and KDP reader acceptance remain separate release gates.
