# Sparse and iterative solvers

Sparse storage avoids zeros when structure and scale justify its index overhead. Iterative methods replace a dense factorization with matrix-vector products and a convergence process. Conjugate gradient requires symmetric positive definiteness; preconditioning, residual criteria, and a maximum iteration count belong in the contract.

**Use it when:** large structured systems make dense storage or factorization impractical.

## Prerequisites

[[Linear systems and fundamental subspaces]], [[Symmetric positive-definite matrices and Cholesky]], [[Matrix norms and numerical error]].

## Used by

[[Matrix factorization choices]], [[Matrix storage indexing and strides]].

**Teaching path:** [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Sparse problems change the economics of solving|sparse and iterative methods]].
