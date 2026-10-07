---
tags:
  - concept
  - foundation
---

# Numerical decision boundaries

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

A small arithmetic perturbation can cross a discrete threshold. Values just below/above pixel midpoint 2.5 choose indices 2/3 under llround; a near-zero field sign can change mesh connectivity. Scalar error and topology are different outputs.

## Prerequisites

- [[Absolute and relative error]] — Discrepancy, magnitude and reference units define sensitivity and threshold significance.
- [[Directed rounding modes]] — Integer/pixel rounding policy controls discrete boundary choices.

## Used by

Connect later dependent concepts here when their lessons arrive.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Robust predicates, classification uncertainty, pixel/voxel bounds and surface-extraction guarantees.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
