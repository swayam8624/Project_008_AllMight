---
tags:
  - concept
  - foundation
---

# Machine epsilon

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

For these binary types epsilon is upward spacing at 1: 2^-23 for binary32 and 2^-52 for binary64. It is neither minimum positive value nor a physical threshold.

## Prerequisites

- [[Significand and hidden bit]] — Fraction weights and effective precision determine value and the smallest retained position.

## Used by

- [[ULP and representable spacing]] — uses this mechanism in its explanation.
- [[Numerical tolerance policies]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Explain min, denorm_min, lowest, and task-specific tolerance independently.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]

## Epsilon is local to one magnitude

For a floating type, machine epsilon is commonly defined as the distance from $1$ to the next representable number greater than $1$. For binary precision $p$, that spacing is $2^{1-p}$. It describes one location in the representable lattice; it is **not** a universal comparison tolerance.

Floating spacing scales with exponent. Near a large world coordinate, one ULP can be much larger than epsilon; near zero, subnormal spacing follows a different regime. Therefore testing

`abs(a-b) < epsilon`

for arbitrary geometric or physical quantities is usually unjustified.

A defensible comparison begins with the domain: absolute tolerance for quantities near zero, relative tolerance when scale matters, ULP bounds when representation-level proximity is intended, or an exact predicate when the algorithm can provide one. Tests should include values near 1, large magnitudes, tiny normals/subnormals and adjacent representable values using `nextafter`.

