---
tags:
  - concept
  - foundation
---

# Floating-point addition and absorption

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

Addition aligns significands to a common exponent before adding/subtracting and rounding. At 2^24, upward binary32 spacing is 2, so adding 1 is a tie absorbed to the same even endpoint. 1.5+0.15625 aligns by three bits and gives 1.65625 exactly.

## Prerequisites

- [[Significand and hidden bit]] — Significant digit positions determine alignment and what leading digits cancel.
- [[Exponent bias]] — Effective exponents must be decoded before comparing scale and shifting.
- [[Guard round and sticky bits]] — Aligned-away information must survive sufficiently to choose the correct rounded neighbor.

## Used by

- [[Floating-point reassociation]] — uses this mechanism to establish its numerical contract.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** A full software adder must handle classes, signs, range and every rounding mode; current trace is not that implementation.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
