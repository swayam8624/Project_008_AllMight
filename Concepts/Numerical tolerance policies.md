---
tags:
  - concept
  - foundation
---

# Numerical tolerance policies

**Curriculum depth:** Developed · **First seen:** C004 · **Latest development:** C005

## Definition and mechanism

Tolerance is a task-chosen allowed discrepancy: absolute in original units, relative to scale, or representable-step based. Machine epsilon is not automatically a norm cutoff. Handle non-finite values and validate tolerances before formulas.

M005 distinguishes absolute tolerance in the quantity's units from dimensionless relative tolerance. Validate policy parameters and classify non-finite values before arithmetic; approximate closeness is not transitive.

## Prerequisites

- [[Machine epsilon]] — Spacing near one supplies a reference precision scale, not a universal local gap or tolerance.
- [[ULP and representable spacing]] — Adjacent representable points define rounding candidates, errors and step distances.
- [[Floating-point infinities]] — Non-finite operands and overflowing intermediates need branches before ordinary finite formulas.
- [[NaNs and payloads]] — Unordered comparisons and payload distinctions require explicit metric, comparison and serialization policy.

## Used by

- [[Conditioning and singularity]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Extend this mechanism to later algorithms and target-specific behavior. See the M005 chapter for its current examples and validity assumptions.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 development]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
