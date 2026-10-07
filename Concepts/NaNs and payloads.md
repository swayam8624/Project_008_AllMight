---
tags:
  - concept
  - foundation
---

# NaNs and payloads

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

All-one exponent with nonzero fraction encodes a non-numerical, unordered NaN. Quiet and signaling classes differ; fraction bits can carry payload information. Ordinary NaN equality is false; use isnan. Raw words can be classified without consuming signaling NaNs as floating values.

## Prerequisites

- [[Binary32]] — The concrete fields provide the representation used in this mechanism.

## Used by

- [[Floating-point environment and compiler modes]] — uses this mechanism in its explanation.
- [[ULP distance policies]] — uses this mechanism in its explanation.
- [[Numerical tolerance policies]] — uses this mechanism in its explanation.
- [[Floating-point canonicalization]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Payload/signaling preservation through arithmetic, parameter passing and printing is not guaranteed by a raw-word classifier.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
