---
tags:
  - concept
  - foundation
---

# Binary64

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

A 64-bit binary interchange format with 1/11/52 fields, 53 significant normal bits and bias 1023. Normal exponent range is −1022 through 1023. Word 3FF0000000000000 means 1.

## Prerequisites

- [[Binary32]] — The concrete fields provide the representation used in this mechanism.

## Used by

- [[Mixed-precision state updates]] — A wider format can represent finite binary32 values exactly but not recover earlier loss.

Connect later dependent concepts here as their lessons arrive.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Apply the class rules using exponent 2047; long double is not automatically this format.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]

## Precision, exponent range, and the integer-exact region

IEEE-754 binary64 uses 1 sign bit, 11 exponent bits, and 52 stored fraction bits. Normal values have an implicit leading one, giving 53 bits of significand precision. Around magnitude one, the gap between adjacent values is $2^{-52}$.

Every integer with magnitude up to $2^{53}$ is exactly representable, but beyond that boundary adjacent integers begin sharing the same floating representation. This matters when timestamps, IDs, byte counts or large counters are moved through a double-based API: an integer may be within the exponent range yet still lose unit precision.

Binary64 dramatically reduces rounding error compared with binary32 but doubles storage/bandwidth and does not cure unstable algorithms. Coordinate systems for large worlds often use double on the CPU and camera-relative float on the GPU, making the conversion boundary part of the design.

Tests should cover adjacent values around $2^{53}$, normal/subnormal transitions, overflow, signed zero, infinities/NaNs, and cancellation cases that remain ill-conditioned even at 64-bit precision.

