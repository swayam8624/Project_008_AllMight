---
tags:
  - concept
  - foundation
---

# Bitwise AND

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

Preserves selected lanes where the mask has 1 and clears lanes where it has 0.

Per lane: 0&0=0, 0&1=0, 1&0=0, 1&1=1. `0xAD & 0x07 = 5`. Testing `(flags & mask)!=0` asks whether any selected bit is set; equality with mask asks whether all are set. For an empty mask, any is false and all is true.

## Prerequisites

- [[Boolean algebra]] — The logical domain is false/true.

## Used by

- [[Logical versus bitwise operators]] — uses bitwise and as part of its representation or reasoning.
- [[Half adder]] — uses bitwise and as part of its representation or reasoning.
- [[Carry-lookahead and parallel-prefix adders]] — uses bitwise and as part of its representation or reasoning.

## Recall and next depth

Explain bitwise and without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** instruction mapping, vector lanes, and atomic read-modify-write.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
