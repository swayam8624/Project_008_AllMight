---
tags:
  - concept
  - foundation
---

# Unit roundoff

**Curriculum depth:** Developed · **First seen:** C004 · **Latest development:** C005

## Definition and mechanism

Under nearest binary rounding the conventional relative bound for normal-range results is u=2^-p, half upward epsilon. For binary32 this is 2^-24. The bound has range/rounding assumptions.

Repeated-rounding bounds use $\gamma_n=nu/(1-nu)$ when $nu<1$, under the stated rounding and range assumptions. Summation error also depends on operand magnitudes; this is not a universal relative-error guarantee.

## Prerequisites

- [[ULP and representable spacing]] — Adjacent representable points define rounding candidates, errors and step distances.
- [[Rounding to nearest ties to even]] — Midpoint parity determines discarded-tail decisions and the normal relative-error bound.

## Used by

- [[Absolute and relative error]] — The local normal-rounding bound supplies the error-analysis scale.
- [[Accumulated rounding error]] — The local normal-rounding bound supplies the error-analysis scale.

Connect later dependent concepts here as their lessons arrive.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Subnormal absolute-error models and target-specific arithmetic require separate treatment; do not extend the normal relative bound to arbitrarily tiny results.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 development]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
