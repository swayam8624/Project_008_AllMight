---
tags:
  - concept
  - foundation
---

# Mixed-precision state updates

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

Finite binary32 promotes exactly to binary64, but lost bits are not recovered. Double update arithmetic followed by float persistent storage rounds each step. A three-sample mean near 0.5 distinguishes float from double state despite double intermediates.

## Prerequisites

- [[Binary32]] — Its finite significand/exponent makes each stored finite value a dyadic rational.
- [[Binary64]] — A wider format can represent finite binary32 values exactly but not recover earlier loss.
- [[Rounding to nearest ties to even]] — Midpoint decisions determine separate/fused results and narrowed persistent state.

## Used by

- [[Capped weighted updates]] — uses this mechanism to establish its numerical contract.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Quantized state, large/rounded weights, overflow-proof generic updates and real reconstruction datasets.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
