# LU factorization and reusable solves

LU stores elimination as triangular factors. With row pivoting in this vault's convention, $PA=LU$; apply the same permutation to each right-hand side, then solve one lower- and one upper-triangular system. Factorization cost is amortized across repeated solves.

**Use it when:** solving square general systems, especially with multiple right-hand sides.

**Developed in Phase 8:** The multipliers used to eliminate entries become the lower factor, while the remaining upper triangle records the reduced system; with row exchanges the convention is $PA=LU$. The runnable pivoted LU reference traces permutation, factor storage, status, and repeated forward/back substitution on actual values. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A runnable pivoted LU reference preserves the elimination history|runnable pivoted LU lab]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Triangular solves perform the cheap second half|forward and backward solves]]

## Prerequisites

[[Gaussian elimination and pivoting]], [[Matrix storage indexing and strides]].

## Used by

[[Matrix factorization choices]], [[Sparse and iterative solvers]].

**Teaching path:** [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Elimination turns the geometry into an auditable calculation|factor and solve]].
