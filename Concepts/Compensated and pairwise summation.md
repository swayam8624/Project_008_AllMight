---
tags:
  - concept
  - foundation
---

# Compensated and pairwise summation

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

Kahan tracks an estimate of lost low-order information; fixed pairwise sums combine balanced halves. For float inputs [2^24,1,1], naive gives 2^24 while both illustrated improved methods reach 2^24+2. Neither is universally exact or order-independent.

## Prerequisites

- [[Floating-point reassociation]] — Different sum groupings define the numerical behavior of reduction methods.
- [[Accumulated rounding error]] — Path-length and magnitude bounds motivate shorter trees and compensation.

## Used by

- [[Deterministic reduction tree]] — uses this mechanism to establish its numerical contract.
- [[Dot and cross-product rounding]] — uses this mechanism to establish its numerical contract.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Adversarial data, non-finite contracts, allocation/parallel costs and stronger accumulators.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
