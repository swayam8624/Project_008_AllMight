---
tags:
  - concept
  - foundation
---

# Numerical tolerance policies

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

Tolerance is a task-chosen allowed discrepancy: absolute in original units, relative to scale, or representable-step based. Machine epsilon is not automatically a norm cutoff. Handle non-finite values and validate tolerances before formulas.

## Prerequisites

- [[Machine epsilon]] — Spacing near one supplies a reference precision scale, not a universal local gap or tolerance.
- [[ULP and representable spacing]] — Adjacent representable points define rounding candidates, errors and step distances.
- [[Floating-point infinities]] — Non-finite operands and overflowing intermediates need branches before ordinary finite formulas.
- [[NaNs and payloads]] — Unordered comparisons and payload distinctions require explicit metric, comparison and serialization policy.

## Used by

- [[Conditioning and singularity]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Specify units, error budget and failure policy; M005 develops error-informed choices.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
