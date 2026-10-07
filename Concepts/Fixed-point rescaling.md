---
tags:
  - concept
  - foundation
---

# Fixed-point rescaling

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

Raw multiplication changes scale S to S²; divide the wide product by S with a declared rounding rule. Raw division uses A*S/B. Check the destination after widening, and handle divisor zero and signed minima.

## Prerequisites

- [[Fixed-point representation]] — supplies the representation or validity rule used above.
- [[Minimum signed value]] — supplies the representation or validity rule used above.
- [[Arithmetic policies]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** General arbitrary-width arithmetic is not supplied by this bounded int32/int64 lab.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]
