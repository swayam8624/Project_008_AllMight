---
tags:
  - concept
  - foundation
---

# Floating-point exception flags

**Curriculum depth:** Developed · **First seen:** C004 · **Latest development:** C005

## Definition and mechanism

Invalid, divide-by-zero, overflow, underflow and inexact are status conditions, not C++ throw exceptions. Exact subnormal results need not signal underflow. Flag/trap behavior depends on the environment.

The strict host lab observes arithmetic flags at runtime and restores the incoming environment. Exactly represented subnormal results need not signal underflow; flags are not a whole-algorithm error estimate.

## Prerequisites

- [[IEEE-754 floating point]] — The format/arithmetic contract gives these bits their numerical domain.
- [[Subnormals and gradual underflow]] — Constant tiny spacing and loss of relative precision define boundary and environment behavior.

## Used by

Connect later dependent concepts here as their lessons arrive.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Extend this mechanism to later algorithms and target-specific behavior. See the M005 chapter for its current examples and validity assumptions.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 development]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
