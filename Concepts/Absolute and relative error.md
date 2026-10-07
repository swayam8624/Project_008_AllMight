---
tags:
  - concept
  - foundation
---

# Absolute and relative error

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

Absolute error |computed−reference| has the reference's units. Relative error divides by nonzero reference magnitude and is dimensionless; it is undefined at true zero. A 1e-12 absolute error can be 100% of a 1e-12 value.

## Prerequisites

- [[Unit roundoff]] — The local normal-rounding bound supplies the error-analysis scale.

## Used by

- [[Affine quantization and zero point]] — uses this mechanism to define its representation, error or validation contract.
- [[Quantization error metrics]] — uses this mechanism to define its representation, error or validation contract.

- [[Forward and backward error]] — uses this mechanism to establish its numerical contract.
- [[Accumulated rounding error]] — uses this mechanism to establish its numerical contract.
- [[Cancellation and loss of significance]] — uses this mechanism to establish its numerical contract.
- [[Numerical decision boundaries]] — uses this mechanism to establish its numerical contract.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Choose an independent reference and handle near-zero/error-budget decisions explicitly.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
