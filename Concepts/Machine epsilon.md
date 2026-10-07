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

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
