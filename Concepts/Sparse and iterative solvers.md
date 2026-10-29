# Sparse and iterative solvers

Sparse storage avoids zeros when structure and scale justify its index overhead. Iterative methods replace a dense factorization with matrix-vector products and a convergence process. Conjugate gradient requires symmetric positive definiteness; preconditioning, residual criteria, and a maximum iteration count belong in the contract.

**Use it when:** large structured systems make dense storage or factorization impractical.

**Developed in Phase 8:** Sparse storage pays off when zeros and dimensions outweigh indexing overhead, while direct factorization can create fill that destroys sparsity. Iterative methods instead use matrix-vector products and stopping tests; conjugate gradient requires an SPD operator, and preconditioning, a maximum iteration count, and a true residual make convergence reviewable. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Sparse storage changes the cost model|storage and fill]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Direct and iterative solvers suit different workloads|direct versus iterative solving]]

## Prerequisites

[[Linear systems and fundamental subspaces]], [[Symmetric positive-definite matrices and Cholesky]], [[Matrix norms and numerical error]].

## Used by

[[Matrix factorization choices]], [[Matrix storage indexing and strides]].

**Teaching path:** [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Sparse problems change the economics of solving|sparse and iterative methods]].
