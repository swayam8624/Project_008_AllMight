---
tags:
  - concept
  - foundation
---

# Numerical reproducibility

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

Repeatability in a defined environment, bitwise identity across a stated environment set, and numerical accuracy are different goals. Atomic updates can be race-free yet ordered differently; a fixed tree alone does not control FMA, subnormals or math libraries.

## Prerequisites

- [[Deterministic reduction tree]] — Repeatable grouping is one necessary component of reproducibility.
- [[Floating-point environment and compiler modes]] — Rounding, contraction, subnormal and optimization assumptions must match.
- [[Atomic operations and write contention]] — Indivisible updates do not impose a unique schedule-independent ordering.

## Used by

Connect later dependent concepts here when their lessons arrive.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Real CPU/GPU/thread-count experiments need recorded build/environment contracts; none is implied by this host lab.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
