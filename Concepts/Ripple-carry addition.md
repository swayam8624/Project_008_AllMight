---
tags:
  - concept
  - foundation
---

# Ripple-carry addition

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

Each full adder feeds its carry to the next position, making worst-case circuit propagation depth grow with word width.

Connect carry-out at bit i to carry-in at i+1. For `01111111 + 1`, a carry traverses the low seven stages. Circuit delay grows with width in this simple design; fixed-width software addition still counts as O(1) with respect to input size.

## Prerequisites

- [[Full adder]] — A full adder adds A, B, and carry-in.

## Used by

- [[Carry-lookahead and parallel-prefix adders]] — uses ripple-carry addition as part of its representation or reasoning.

## Recall and next depth

Explain ripple-carry addition without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** physical timing, layout, and faster carry networks.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
