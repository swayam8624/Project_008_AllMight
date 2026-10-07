---
tags:
  - concept
  - foundation
---

# Dot and cross-product rounding

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

Dot sums products with intermediate rounding; cross components subtract products and can cancel. In the intended (1e8,1e8) and (1e8,1e8+1) float example, +1 is lost at input conversion before products. FMA/widening cannot recover discarded input.

## Prerequisites

- [[Cancellation and loss of significance]] — Near-equal operand errors motivate exact-subtraction distinctions and reformulation.
- [[Fused multiply-add]] — Contraction changes intermediate rounding in a multiplication/addition graph.
- [[Compensated and pairwise summation]] — A fixed pairwise topology specifies the additions a deterministic reduction performs.

## Used by

Connect later dependent concepts here when their lessons arrive.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Robust/adaptive geometry and verified live implementation/assembly; reported Kairo excerpts remain unverified.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
