---
tags:
  - concept
  - foundation
---

# Fixed-point representation

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

A raw integer I represents I/S for an agreed positive scale S and units. With S=2^16, raw 212992 means 3.25. The integer has no built-in fractional point.

## Prerequisites

- [[Two's complement]] — supplies the representation or validity rule used above.
- [[Interpretation contract]] — supplies the representation or validity rule used above.

## Used by

- [[Fixed-point range and resolution]] — uses this mechanism to define its representation, error or validation contract.
- [[Fixed-point rescaling]] — uses this mechanism to define its representation, error or validation contract.
- [[Affine quantization and zero point]] — uses this mechanism to define its representation, error or validation contract.

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Nonbinary scale APIs and application-specific fixed formats remain future depth.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]
