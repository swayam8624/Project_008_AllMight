---
tags:
  - concept
  - foundation
---

# Unit roundoff

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

Under nearest binary rounding the conventional relative bound for normal-range results is u=2^-p, half upward epsilon. For binary32 this is 2^-24. The bound has range/rounding assumptions.

## Prerequisites

- [[ULP and representable spacing]] — Adjacent representable points define rounding candidates, errors and step distances.
- [[Rounding to nearest ties to even]] — Midpoint parity determines discarded-tail decisions and the normal relative-error bound.

## Used by

Connect later dependent concepts here as their lessons arrive.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Do not apply the same relative bound to arbitrarily tiny subnormal results.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
