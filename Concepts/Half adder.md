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

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Arithmetic becomes a policy|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]

## From truth table to gate delay

A half adder adds two one-bit inputs $a,b$. The sum bit is XOR and the carry bit is AND:

$$
s=a\oplus b,\qquad c=a\land b.
$$

The truth table is not just a memorization device; it proves why one output bit cannot encode all four input sums. Input $1+1$ produces binary $10$, requiring a second bit for carry.

A half adder has no carry-in, so it cannot by itself build every bit position of a multi-bit integer addition. A full adder incorporates carry-in and can be composed into ripple-carry or faster prefix structures. The important systems connection is dependency depth: in a ripple chain, each bit may wait for the previous carry, whereas parallel-prefix adders reorganize generate/propagate information to reduce critical path.

A reconstruction exercise should derive XOR/AND from the four rows, compose two half adders into a full adder, then explain why gate count and propagation depth are different optimization targets.

