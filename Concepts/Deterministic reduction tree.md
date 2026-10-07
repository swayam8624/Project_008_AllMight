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

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
