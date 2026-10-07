---
tags:
  - concept
  - foundation
---

# Signed zero

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

Exponent and fraction zero permit either sign. 00000000 and 80000000 compare numerically equal, but signbit and reciprocal distinguish them. Ordinary x<0 does not detect negative zero.

## Prerequisites

- [[Binary32]] — The concrete fields provide the representation used in this mechanism.

## Used by

- [[ULP distance policies]] — uses this mechanism in its explanation.
- [[Floating-point canonicalization]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Specify zero equality separately from raw-byte equality and ULP ordering.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
