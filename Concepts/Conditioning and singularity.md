---
tags:
  - concept
  - foundation
---

# Conditioning and singularity

**Curriculum depth:** Developed · **First seen:** C004 · **Latest development:** Phase7

## Definition and mechanism

Conditioning measures sensitivity to input perturbation. $A=10^{-4}I_2$ has determinant $10^{-8}$ but condition number one in the 2-norm. Uniform scaling changes determinant magnitude without changing this conditioning measure.

Phase 7 develops $\kappa(A)=\|A\|\|A^{-1}\|$, near-parallel-column sensitivity, pivoted elimination and inverse residuals. Equal determinants can conceal very different stretch ratios: $\operatorname{diag}(10^{-10},10^{-10})$ and $\operatorname{diag}(1,10^{-20})$ both have determinant $10^{-20}$ but conditions one and $10^{20}$. [[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions#An inverse can exist and still amplify uncertainty|Full worked sensitivity example]].

Subtraction of nearby inputs exposes input uncertainty relative to a small difference. Sensitivity belongs to the problem; exact subtraction of stored operands cannot recover information lost when those inputs were rounded.

## Prerequisites

- [[Numerical tolerance policies]] — An absolute numerical threshold can change with scale without measuring mathematical sensitivity.

## Used by
- [[Matrix inversion pivoting and residuals]]

- [[Forward and backward error]] — Input sensitivity is distinct from errors an implementation introduces.
- [[Cancellation and loss of significance]] — Input sensitivity is distinct from errors an implementation introduces.

Connect later dependent concepts here as their lessons arrive.
- [[Gaussian elimination and pivoting]] — Extends this foundation into linear-systems and decomposition analysis.
- [[QR factorization and least squares]] — Extends this foundation into linear-systems and decomposition analysis.
- [[ODE integration and stability]]

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Robust condition estimation, growth factors, advanced factorizations and application-specific error budgets extend the current inversion laboratory.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 development]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
