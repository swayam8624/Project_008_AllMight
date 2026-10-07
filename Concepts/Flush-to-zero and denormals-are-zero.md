---
tags:
  - concept
  - foundation
---

# Flush-to-zero and denormals-are-zero

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

FTZ flushes certain tiny results to zero; DAZ treats subnormal inputs as zero in supporting environments. They change gradual-underflow behavior. Cost and support are target/mode-specific.

## Prerequisites

- [[Subnormals and gradual underflow]] — Constant tiny spacing and loss of relative precision define boundary and environment behavior.
- [[Floating-point environment and compiler modes]] — Compiler/runtime assumptions must preserve the intended direction and special-value semantics.

## Used by

Connect later dependent concepts here as their lessons arrive.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Measure actual shader/SIMD settings before claiming subnormal performance or preservation.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
