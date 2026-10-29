# Gaussian elimination and pivoting

Equivalent elementary row operations expose pivots and triangular structure while preserving the solution set when applied to the full augmented system. Pivoting avoids zero or dangerously small divisors; it improves the calculation but does not cure an ill-conditioned problem.

**Use it when:** understanding REF/RREF, LU, consistency, rank, and triangular solves.

**Developed in Phase 8:** Elimination turns those geometric questions into auditable row operations on the augmented matrix. Pivots expose rank and free variables; partial pivoting chooses a safer divisor and tracks row exchanges, while residual and growth checks explain why pivoting cannot repair ill-conditioning. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Elimination turns the geometry into an auditable calculation|elimination steps]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Pivoting separates algorithm failure from matrix failure|pivoting and growth]]

## Prerequisites

[[Linear systems and fundamental subspaces]], [[Matrix multiplication and composition order]], [[Conditioning and singularity]].

## Used by

[[LU factorization and reusable solves]], [[Matrix norms and numerical error]], [[Sparse and iterative solvers]].

**Teaching path:** [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Elimination turns the geometry into an auditable calculation|elimination story]].
- [[Matrix factorization choices]] — Extends this foundation into the Phase 8 dependency spine.
- [[Conditional Gaussians and Schur complements]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#5. Waiting times, event counts, and covariance geometry|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · Phase 10 visual recall
