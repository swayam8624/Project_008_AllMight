---
tags: [concept, phase-7]
---

# Matrix storage indexing and strides

**Curriculum depth:** Introduced · **First seen:** Phase7 · **Latest development:** Phase7

## Definition and mechanism

Matrix row/column semantics are independent of physical row-major or column-major storage. A strided view uses explicit row and column strides, shape, backing extent, and borrow lifetime. Separate named scalar members are not an array eligible for pointer indexing.

**Reconstruction cue:** For row-major strides row=8 and column=1, element (2,3) has scalar offset 19.

## Prerequisites

- [[ABI and object layout]]
- [[Pointer arithmetic and provenance]]
- [[Matrices as linear maps and basis images]]

## Used by

- [[Matrix SIMD tiling and cache reuse]]
- [[LU factorization and reusable solves]] — Extends this foundation into linear-systems and decomposition analysis.
- [[Sparse and iterative solvers]] — Extends this foundation into linear-systems and decomposition analysis.

- [[GPU vertex specification]]
## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]
