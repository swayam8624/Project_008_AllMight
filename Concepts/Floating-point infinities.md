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

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
