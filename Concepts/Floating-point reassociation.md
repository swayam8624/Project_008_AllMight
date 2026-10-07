---
tags:
  - concept
  - foundation
---

# Floating-point reassociation

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

Changing grouping moves rounding points. With binary32 1e20,−1e20,3.14, groupings give 3.14f versus zero. Distributive factored/expanded paths can also differ: the supplied strict witness gives 1192.0928955078125 versus 1024.

## Prerequisites

- [[Floating-point addition and absorption]] — Lost small addends explain why a regrouped graph can return a different result.
- [[Fused multiply-add]] — Contraction changes intermediate rounding in a multiplication/addition graph.

## Used by

- [[Compensated and pairwise summation]] — uses this mechanism to establish its numerical contract.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Source syntax, compiler permissions, contraction and generated reductions must be distinguished.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
