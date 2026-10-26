# Matrix factorization choices

Choose a solver from the matrix promises and task: pivoted LU for general square systems, Cholesky for SPD systems, QR for full-rank least squares, SVD or rank-revealing methods when weak directions matter, and iterative methods for suitable large sparse workloads. Reuse factors for repeated right-hand sides.

**Use it when:** choosing an algorithm before implementing or calling a numerical routine.

## Prerequisites

[[Gaussian elimination and pivoting]], [[LU factorization and reusable solves]], [[Symmetric positive-definite matrices and Cholesky]], [[QR factorization and least squares]], [[Singular value decomposition]], [[Sparse and iterative solvers]].

## Used by

[[Sparse and iterative solvers]], [[Matrix functions]].

**Teaching path:** [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Close the journey with an algorithm-selection conversation|selection conversation]].
