---
tags:
  - concept
  - foundation
---

# Deterministic reduction tree

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

A reduction fixes input order, partition and merge topology so scheduling cannot change the arithmetic grouping. Midpoint recursive splitting is one such tree. Different SIMD widths or partitions can otherwise change it.

## Prerequisites

- [[Compensated and pairwise summation]] — A fixed pairwise topology specifies the additions a deterministic reduction performs.

## Used by

- [[Numerical reproducibility]] — uses this mechanism to establish its numerical contract.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Cross-platform identity also requires compatible operation semantics, precision, libraries and environment.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]

## Reconstruction depth

Floating-point addition is not associative, so a parallel reduction can change low bits when thread count, work partition or scheduling changes. A deterministic reduction tree fixes grouping order independently of which worker finishes first.

For eight values, a balanced tree can always compute $((x_0+x_1)+(x_2+x_3)) + ((x_4+x_5)+(x_6+x_7))$. The balanced tree can also reduce error growth compared with some highly unbalanced orders, although it is not equivalent to exact or compensated summation.

Determinism costs constraints: the implementation may need stable chunk boundaries, temporary partials and an ordered merge. On GPUs, subgroup width and dispatch shape can otherwise change the effective tree.

Tests should permute scheduling while preserving logical indices and require identical result bits for the deterministic path. A separate accuracy test should compare against a higher-precision or compensated reference; repeatability and numerical accuracy are related but distinct properties.

