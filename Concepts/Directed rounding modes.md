---
tags:
  - concept
  - foundation
---

# Directed rounding modes

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

Upward means toward positive infinity, downward toward negative infinity, and toward-zero truncates magnitude. Upward rounding of −2.1 to an integer gives −2, not −3.

## Prerequisites

- [[Rounding to nearest ties to even]] — Midpoint parity determines discarded-tail decisions and the normal relative-error bound.

## Used by

- [[Floating-point environment and compiler modes]] — uses this mechanism in its explanation.
- [[Interval arithmetic]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Use these signed directions on adjacent floating neighbors, not only integers.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
