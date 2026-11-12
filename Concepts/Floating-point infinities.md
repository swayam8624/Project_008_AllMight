---
tags:
  - concept
  - foundation
---

# Floating-point infinities

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

All-one exponent and zero fraction encode signed infinity. This is not maximum finite. Infinity minus itself is NaN; overflow's result also depends on rounding direction.

## Prerequisites

- [[Binary32]] — The concrete fields provide the representation used in this mechanism.

## Used by

- [[Numerical tolerance policies]] — uses this mechanism in its explanation.
- [[Robust norm and intermediate range]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Define non-finite input policy before applying a relative tolerance formula.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]

## Reconstruction depth

IEEE-754 infinities represent values beyond the finite exponent range and results such as nonzero finite division by signed zero. They participate in arithmetic according to defined extended-real-like rules. Adding a finite value to positive infinity stays positive infinity, while indeterminate forms such as infinity minus infinity produce NaN.

Infinity can be useful as a sentinel in algorithms, such as an initial shortest-path distance, because comparisons with finite positive values behave naturally. But it is not automatically a valid physical or engine state. A transform position, mass, texture coordinate or timestamp becoming infinite usually signals overflow or invalid input and should be caught at a subsystem boundary.

A robust numeric API distinguishes "the floating type can encode infinity" from "this domain accepts infinity." Tests should exercise overflow, division by zero under the active floating environment, comparisons, finite-value validation and serialization behavior. Fast-math modes may weaken exceptional-value assumptions, so production guarantees must match compile configuration.

