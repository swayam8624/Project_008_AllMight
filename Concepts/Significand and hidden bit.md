---
tags:
  - concept
  - foundation
---

# Significand and hidden bit

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

The significand supplies significant digits; the exponent supplies scale. A normal binary value begins 1.f, so its leading one is implied, not stored. Binary32 fraction F contributes F/2^23; subnormals instead begin 0.f.

## Prerequisites

- [[Binary32]] — The concrete fields provide the representation used in this mechanism.
- [[Positional notation]] — Negative powers of two give fractional digits their exact weights.

## Used by

- [[Floating-point addition and absorption]] — Significant digit positions determine alignment and what leading digits cancel.
- [[Cancellation and loss of significance]] — Significant digit positions determine alignment and what leading digits cancel.

- [[Floating-point encoding and decoding]] — uses this mechanism in its explanation.
- [[Subnormals and gradual underflow]] — uses this mechanism in its explanation.
- [[Machine epsilon]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Explain why 23 fraction bits give 24 normal bits but not 24-bit relative precision arbitrarily close to zero.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
