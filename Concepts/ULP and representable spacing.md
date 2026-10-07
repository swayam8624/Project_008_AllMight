---
tags:
  - concept
  - foundation
---

# ULP and representable spacing

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

A unit in the last place describes local spacing. Inside normal binade [2^e,2^(e+1)), p-bit precision gives spacing 2^(e-(p-1)). Immediately above 1 the binary32 gap is twice the gap below it.

## Prerequisites

- [[Machine epsilon]] — Spacing near one supplies a reference precision scale, not a universal local gap or tolerance.
- [[Exponent bias]] — Stored exponents must be translated to effective powers before reconstructing value or spacing.
- [[Subnormals and gradual underflow]] — Constant tiny spacing and loss of relative precision define boundary and environment behavior.

## Used by

- [[Rounding to nearest ties to even]] — uses this mechanism in its explanation.
- [[Unit roundoff]] — uses this mechanism in its explanation.
- [[ULP distance policies]] — uses this mechanism in its explanation.
- [[Numerical tolerance policies]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** State a direction/boundary convention; infinity is not a finite next-neighbor gap.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
