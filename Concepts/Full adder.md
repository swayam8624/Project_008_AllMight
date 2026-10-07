---
tags:
  - concept
  - foundation
---

# Full adder

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

Adds $A,B,C_{in}$; sum is three-way XOR and carry is the majority of its inputs.

A full adder adds A, B, and carry-in. Sum is their parity; carry-out is 1 when at least two inputs are 1. Build it from two half adders: add A,B, then add that sum to carry-in, then OR the two carry outputs.

## Prerequisites

- [[Half adder]] — A half adder adds two bits with sum S=A XOR B and carry C=A AND B.
- [[Carry and borrow]] — For N-bit unsigned addition, carry-out means the exact sum exceeds 2ᴺ−1.

## Used by

- [[Ripple-carry addition]] — uses full adder as part of its representation or reasoning.
- [[Carry, overflow, negative, and zero flags]] — uses full adder as part of its representation or reasoning.
- [[Subtraction through addition]] — uses full adder as part of its representation or reasoning.
- [[Shift-and-add multiplication]] — uses full adder as part of its representation or reasoning.

## Recall and next depth

Explain full adder without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** Apply and reconstruct this concept independently.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
