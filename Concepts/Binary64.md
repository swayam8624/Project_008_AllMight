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

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
