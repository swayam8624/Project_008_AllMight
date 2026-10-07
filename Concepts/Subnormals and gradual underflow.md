---
tags:
  - concept
  - foundation
---

# Subnormals and gradual underflow

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

Exponent zero and nonzero fraction remove the hidden one and hold the effective exponent at the normal minimum. Binary32 values are F×2^-149, connecting smoothly to 2^-126. Absolute spacing stays fixed while relative precision degrades.

## Prerequisites

- [[Exponent bias]] — Stored exponents must be translated to effective powers before reconstructing value or spacing.
- [[Significand and hidden bit]] — Fraction weights and effective precision determine value and the smallest retained position.

## Used by

- [[ULP and representable spacing]] — uses this mechanism in its explanation.
- [[Floating-point exception flags]] — uses this mechanism in its explanation.
- [[Flush-to-zero and denormals-are-zero]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** A subnormal value alone does not imply an underflow flag; distinguish tininess from inexactness.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
