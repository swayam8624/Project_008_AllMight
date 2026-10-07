---
tags:
  - concept
  - foundation
---

# Floating-point exception flags

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

Invalid, divide-by-zero, overflow, underflow and inexact are status conditions, not C++ throw exceptions. Exact subnormal results need not signal underflow. Flag/trap behavior depends on the environment.

## Prerequisites

- [[IEEE-754 floating point]] — The format/arithmetic contract gives these bits their numerical domain.
- [[Subnormals and gradual underflow]] — Constant tiny spacing and loss of relative precision define boundary and environment behavior.

## Used by

Connect later dependent concepts here as their lessons arrive.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** M005 will relate flags to complete arithmetic operations; current lab is not a full flag oracle.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
