---
tags:
  - concept
  - foundation
---

# Capped weighted updates

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

A cap can discard old influence mathematically. With cap 2 and unit-weight observations [0,1,−1] versus [0,−1,1], exact distances end at −0.25 versus +0.25. This is not merely floating summation order.

## Prerequisites

- [[Mixed-precision state updates]] — Persistent rounding must be separated from mathematical history-weight policy.
- [[Arithmetic policies]] — Capping changes which influence is retained rather than only the representation.

## Used by

Connect later dependent concepts here when their lessons arrive.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Changing to batch averaging changes the algorithm; separate history policy from rounding.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
