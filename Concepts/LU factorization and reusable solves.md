# LU factorization and reusable solves

LU stores elimination as triangular factors. With row pivoting in this vault's convention, $PA=LU$; apply the same permutation to each right-hand side, then solve one lower- and one upper-triangular system. Factorization cost is amortized across repeated solves.

**Use it when:** solving square general systems, especially with multiple right-hand sides.

## Prerequisites

[[Gaussian elimination and pivoting]], [[Matrix storage indexing and strides]].

## Used by

[[Matrix factorization choices]], [[Sparse and iterative solvers]].

**Teaching path:** [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Elimination turns the geometry into an auditable calculation|factor and solve]].
