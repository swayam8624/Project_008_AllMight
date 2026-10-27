# Matrix factorization choices

Choose a solver from the matrix promises and task: pivoted LU for general square systems, Cholesky for SPD systems, QR for full-rank least squares, SVD or rank-revealing methods when weak directions matter, and iterative methods for suitable large sparse workloads. Reuse factors for repeated right-hand sides.

**Use it when:** choosing an algorithm before implementing or calling a numerical routine.

**Developed in Phase 8:** The factorization should follow the matrix structure and the requested result: LU handles general square systems, Cholesky exploits positive definiteness, QR fits full-rank least squares, and SVD handles uncertain rank and spectral questions. The comparison also weighs sparsity, reuse, memory, and whether an iterative tolerance is acceptable. [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Choose a factorization from structure and the requested answer|factorization selection]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#A runnable pivoted LU reference preserves the elimination history|the runnable LU example]]

## Prerequisites

[[Gaussian elimination and pivoting]], [[LU factorization and reusable solves]], [[Symmetric positive-definite matrices and Cholesky]], [[QR factorization and least squares]], [[Singular value decomposition]], [[Sparse and iterative solvers]].

## Used by

[[Sparse and iterative solvers]], [[Matrix functions]].

**Teaching path:** [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Close the journey with an algorithm-selection conversation|selection conversation]].
