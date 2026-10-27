# Linear systems and fundamental subspaces

A linear system $A\mathbf x=\mathbf b$ asks which inputs satisfy all row measurements at once. It is consistent exactly when $\mathbf b$ lies in the column space. The null space contains input changes invisible to every measurement; rank-nullity counts observable versus free directions.

**Use it when:** explaining consistency, underdetermination, least squares, parameter observability, or constraints in geometry.

**Developed in Phase 8:** The expanded treatment connects each row to a hyperplane and the complete matrix to a map: consistency asks whether the target lies in the column space, while the null space describes every invisible change to a solution. Rank then counts constrained directions and determines the dimension of the solution family. [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#A system is an intersection of constraints|constraint geometry]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Rank, the four spaces, and the shape of the answer|rank and solution families]]

## Prerequisites

[[Matrices as linear maps and basis images]], [[Basis coordinates and dimension]], [[Projection rejection and scalar components]].

## Used by

[[Gaussian elimination and pivoting]], [[QR factorization and least squares]], [[Singular value decomposition]], [[Moore-Penrose pseudoinverse]], [[Sparse and iterative solvers]].

**Teaching path:** [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#A linear equation is a measurement, and several equations are geometry|constraint geometry]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Rank separates what the measurements determine from what they leave free|four spaces]].
- [[Eigenvalues and eigenspaces]] — Extends this foundation into the Phase 8 dependency spine.
