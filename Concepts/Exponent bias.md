---
tags:
  - concept
  - foundation
---

# Exponent bias

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

Normal exponents are encoded as unsigned E=e+B. Binary32 B=127, so exponent 3 stores 130. Field zero and all-ones are reserved; classify before subtracting bias.

## Prerequisites

- [[Binary32]] — The concrete fields provide the representation used in this mechanism.

## Used by

- [[Floating-point addition and absorption]] — Effective exponents must be decoded before comparing scale and shifting.

- [[Floating-point encoding and decoding]] — uses this mechanism in its explanation.
- [[Subnormals and gradual underflow]] — uses this mechanism in its explanation.
- [[ULP and representable spacing]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Generalize B=2^(k-1)-1 to binary64 without mis-decoding subnormals.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
