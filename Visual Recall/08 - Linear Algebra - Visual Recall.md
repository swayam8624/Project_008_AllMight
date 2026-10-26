---
tags:
  - visual-recall
  - linear-algebra
coverage: Parts 701–800
---

# Phase 8 — Linear Algebra: Active Recall

**Fast spine:** Measurements constrain → rank separates knowledge from freedom → factorizations expose structure → eigenvectors describe square-map directions → SVD describes every map → stable APIs report evidence.

Read the full explanations in [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions|From Constraints to Spectra]]. These figure links jump to their source chapters.

## The story at a glance

```mermaid
%%{init: {'theme':'base','themeVariables':{'primaryColor':'#e0f2fe','primaryTextColor':'#15304a','lineColor':'#64748b','fontFamily':'sans-serif'}}}%%
flowchart LR
  C["Constraints Ax = b"] --> R["Rank / spaces"]
  R --> F["Structure-aware factorization"]
  F --> E["Eigenanalysis or SVD"]
  E --> A["Fit, compress, transform"]
  A --> V["Validate residuals + invariants"]
```

## Retrieve the mechanisms

| Cue | Short answer | Read / view |
|---|---|---|
| Why may $A\mathbf x=\mathbf b$ have no answer? | The target is outside $\operatorname{Col}(A)$. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#A linear equation is a measurement, and several equations are geometry|Constraint geometry]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Figure L2 - Consistency, freedom, and the four fundamental spaces|L2]] |
| What does a null vector mean? | An input change no measurement can see; add it to any solution. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Rank separates what the measurements determine from what they leave free|Four spaces]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Figure L2 - Consistency, freedom, and the four fundamental spaces|L2]] |
| What does elimination preserve? | The solution set, only if row operations also transform $\mathbf b$. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Elimination turns the geometry into an auditable calculation|Elimination]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Figure L3 - Elimination stores work for later solves|L3]] |
| Why factor once? | Reuse $L,U$ or $Q,R$ for additional right-hand sides. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Figure L3 - Elimination stores work for later solves|L3]] |
| Which factorization for what structure? | LU general square; Cholesky SPD; QR least squares; SVD rank insight; iteration for huge sparse systems when suitable. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Structure makes a decomposition a better tool than generic elimination|Factorization guide]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Figure L4 - Matrix structure determines the useful factorization|L4]] |
| What does a small residual prove? | Near satisfaction of the equations; sensitivity can still make the solution inaccurate. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Eigenvectors reveal the directions a square map does not turn|Eigenpair sensitivity]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Matrix norms give scale to residuals and error claims|Norms and error]] |
| Why is a symmetric eigenbasis special? | It is orthonormal, preserving 2-norm while diagonalizing the map. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Figure L5 - Eigenanalysis is cleanest when the basis is orthonormal|L5]] |
| What is the SVD action? | Input orthogonal coordinates → independent stretches → output orthogonal coordinates. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Singular value decomposition covers every rectangular map|SVD geometry]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Figure L6 - Every linear map is orthogonal change, stretch, orthogonal change|L6]] |
| What does the pseudoinverse choose? | Exact $A^+$: least-squares fit, then minimum norm. Truncated $A_\tau^+$: solve the retained-rank problem; original residual can increase. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Pseudoinverse makes least squares and minimum norm one operation|Pseudoinverse]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Figure L7 - The singular spectrum makes inversion and compression explicit|L7]] |
| How does PCA connect to SVD? | Center rows, factor $X=U\Sigma V^\mathsf T$; $V$ gives principal directions and $U\Sigma$ scores. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#PCA turns a cloud of points into measured variation|PCA]] |
| What does polar decomposition separate? | Orthogonal motion from symmetric stretch; reflection and singular ambiguity need attention. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Polar decomposition separates rotation-like motion from stretch|Polar decomposition]] |
| Which SVD checks are independent? | Reconstruction, $U$ orthogonality, $V$ orthogonality, ordering, dimensions, convergence. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Figure L8 - SVD validation needs independent checks|L8]] |
| When can diagonalization explain a matrix function? | When its eigenbasis is usable; otherwise use a stable function-specific algorithm. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Matrix functions turn repeated linear actions into one operator|Matrix functions]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Figure L9 - A matrix function follows the same action after a change of basis|L9]] |
| Direct or iterative solve? | Decide from size, sparsity, right-hand-side reuse, convergence, memory, and error needs. | [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Figure L10 - Direct and iterative methods occupy different workloads|L10]] |

## Thirty-second reconstruction

1. Sketch one measurement as a hyperplane; explain consistency using column space.
2. Explain why rank and nullity count determined and invisible directions.
3. Derive one pivot step and show the same row operation on the right-hand side.
4. Select LU, Cholesky, QR, SVD, or an iterative method from matrix structure.
5. Explain why normal equations square the condition number.
6. Trace $A=U\Sigma V^\mathsf T$ on one vector.
7. State what thresholding changes in a pseudoinverse or truncated SVD.
8. Name separate SVD correctness checks and what a residual cannot prove.

[[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions|Full teaching volume]] · [[Supplementary/Foundations#Phase 8 - Parts 701-800|100-part title ledger]] · [[Dependency Map#Phase 8 - From constraints to spectra|Dependency spine]]
