---
tags:
  - concept
  - foundation
---

# Half adder

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

Adds two bits with $S=A\oplus B$ and $C=A\land B$, but accepts no incoming carry.

A half adder adds two bits with sum S=A XOR B and carry C=A AND B. Inputs 1,1 produce binary 10: low sum 0, carry 1. It cannot consume a carry from a previous position, which motivates the full adder.

## Prerequisites

- [[Bitwise XOR]] — XOR returns 1 when inputs differ.
- [[Bitwise AND]] — Per lane: 0&0=0, 0&1=0, 1&0=0, 1&1=1.

## Used by

- [[Full adder]] — uses half adder as part of its representation or reasoning.

## Recall and next depth

Explain half adder without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** Apply and reconstruct this concept independently.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
