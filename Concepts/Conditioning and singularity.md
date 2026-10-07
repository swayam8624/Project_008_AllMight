---
tags:
  - concept
  - foundation
---

# Conditioning and singularity

**Curriculum depth:** Developed · **First seen:** C004 · **Latest development:** C005

## Definition and mechanism

Conditioning measures sensitivity to input perturbation. A=1e-4 I has determinant 1e-8 but condition number one in the 2-norm. Uniform scaling changes determinant magnitude without changing this conditioning measure.

Subtraction of nearby inputs exposes input uncertainty relative to a small difference. Sensitivity belongs to the problem; exact subtraction of stored operands cannot recover information lost when those inputs were rounded.

## Prerequisites

- [[Numerical tolerance policies]] — An absolute numerical threshold can change with scale without measuring mathematical sensitivity.

## Used by

- [[Forward and backward error]] — Input sensitivity is distinct from errors an implementation introduces.
- [[Cancellation and loss of significance]] — Input sensitivity is distinct from errors an implementation introduces.

Connect later dependent concepts here as their lessons arrive.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** General matrix conditioning, pivoting and inversion algorithms await later linear algebra.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 development]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
