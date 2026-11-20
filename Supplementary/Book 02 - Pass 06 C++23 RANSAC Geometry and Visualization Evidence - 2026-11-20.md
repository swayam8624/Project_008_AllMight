# Book Two — Pass 06: Native C++23 Geometric Consensus

**Book:** From Equations to Geometry
**Branch:** \`curriculum/book02-mathematics-refinement-pass01\`
**Date:** 20 November 2026
**Scope:** Another additive source pass, not a full PDF, EPUB, or KDP release.

## Engineering objective

Move an actual computational-geometry project from a Python-dependent
teaching demonstration toward a native C++23 algorithm with mathematical
contracts, outputs that can be audited independently, and a plot driven by
the exact same C++ computation. Do not overwrite the original work or
relabel the earlier scientific figures as native results.

## Implemented files

- \`tools/labs/book02_cpp23/src/geometry.hpp\`: normalized implicit line,
  two-point minimal hypotheses, signed orthogonal point-to-line distance,
  covariance/eigenvector total-least-squares fit, explicit isotropic-cloud
  degeneracy, idealized RANSAC confidence formula, seeded finite-trial
  consensus, one orthogonal refit and recomputed membership, RMS.
- \`tools/labs/book02_cpp23/src/main.cpp\`: native commands
  \`ransac-summary\` and \`ransac-data\`, plus tests covering the exact
  line, probability fixture, isotropic degeneracy, invalid parameters,
  finite RMS, known-recovery tolerance and known-label membership.
- \`tools/labs/book02_cpp23/plot.py\`: optional source-only scatter plot
  of all 110 input observations, selected vs rejected membership, the
  generating model, the RANSAC/TLS fit and the all-point TLS baseline.
  No Python model fit or random sampling is performed.
- \`tools/labs/book02_cpp23/CMakeLists.txt\`: native CTest exports.
- \`tools/labs/book02_cpp23/editorial_gate.py\`: native geometry source,
  command, and Book Two chapter integration invariants.
- \`Book Production/AllMight Codex/LaTeX/Assets/Book02/25-ransac-paper-lab.tex\`:
  Rayleigh-quotient derivation of orthogonal TLS, unit conventions,
  alternative models, sampling assumptions, CSV instructions, and
  explicit methodological limits.
- \`Book Production/AllMight Codex/LaTeX/Assets/Book02/35-cumulative-numerical-geometry-atlas.tex\`:
  native C++23 capstone bridge.

## Exact synthetic experiment

The inlier group has 80 points at \`x=-4+8*i/79\`,
\`y=2*x+1+0.015*sin(0.27*i+0.4)\`; the outlier group has 30
points at \`x=-3.8+7.6*i/29\`,
\`y=-7+0.4*x+0.06*cos(0.51*i)\`. Both \`i\` indices begin at zero
within their own group. The threshold is 0.04 units, budget 400
hypotheses, and random seed 73013. The truth labels are published
with the synthetic data and never inferred from the result.

An independent local C++23 harness under Clang and GCC and a
GCC AddressSanitizer/UBSan harness yielded the correctly normalized
RANSAC line around \`(a,b,c)=(0.894429,-0.447211,0.44782)\`,
80 selected true inliers, 0 false positives and RMS around
0.004746. The all-point TLS line was different, as expected for
this structured outlier scenario. These numbers describe exactly
this fixture and are not a performance or generalization claim.

A local test caught an \`std::move\`-after-size-evaluation hazard in
the first RMS-result initializer: its moved-from membership vector
could report size zero, producing an infinite RMS. The source was
corrected before the header was committed; the acceptance test now
explicitly requires a finite RMS.

## Mac reproduction

\`\`\`bash
git switch curriculum/book02-mathematics-refinement-pass01
git pull --ff-only origin curriculum/book02-mathematics-refinement-pass01
cmake -S tools/labs/book02_cpp23 -B build/book02-cpp23 -DCMAKE_BUILD_TYPE=Release
cmake --build build/book02-cpp23 --parallel
ctest --test-dir build/book02-cpp23 --output-on-failure
build/book02-cpp23/book02_workbench ransac-summary
build/book02-cpp23/book02_workbench ransac-data > build/book02-cpp23/ransac-data.csv
python3 tools/labs/book02_cpp23/plot.py build/book02-cpp23/ransac-data.csv --output build/book02-cpp23/ransac-data.png
\`\`\`

No LaTeX compilation is required for the above; the plot step
requires Matplotlib, but the numerical project itself uses
only standard C++23.

## Verification boundaries and next targets

Native CI should verify all C++ sources, test and export routes and
the Book Two-only additive TeX constraints. The original 16 chapters
remain under Git-blob source lock; all 36 original supplements remain.

The existing Python-led print/EPUB RANSAC plate uses a related but
**different** declared dataset and algorithm implementation. It is
not automatically replaced by the new C++ export. Full figure-source
migration requires validation of native-vs-existing semantics and
the source-driven SVG/EPUB image pipeline, not an unreviewed picture swap.

Next native migrations: general exact binary64 orientation fallback,
triangle/FEM assembly, sparse system solves and heat-method geodesics,
multiple-importance sampling, nonlinear geometry/pose objectives,
independent numerical references, repeated-seed studies and full
publication-quality visual proof on the author's Mac.
