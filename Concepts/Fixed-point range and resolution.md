---
tags:
  - concept
  - foundation
---

# Fixed-point range and resolution

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

For signed N-bit raw storage and F fraction bits, step is 2^-F and the positive endpoint is (2^(N-1)-1)/2^F. Required step 0.001 and magnitude 1e6 in 32 bits allow F=10 or 11.

## Prerequisites

- [[Fixed-point representation]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Coordinate-origin and wider-carrier designs need their own range proofs.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]
