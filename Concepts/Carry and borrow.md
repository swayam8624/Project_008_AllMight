---
tags:
  - concept
  - foundation
---

# Carry and borrow

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

Carry is information leaving the high end of unsigned addition; borrow records an unmet subtraction at a bit position. Architecture subtraction flags may encode borrow or no-borrow.

For N-bit unsigned addition, carry-out means the exact sum exceeds 2ᴺ−1. A borrow means a subtraction needs value from a higher position. In an ordinary unsigned subtract, A<B implies borrow. Some architectures expose the complement, 'no borrow', in their carry flag.

## Prerequisites

- [[Positional notation]] — A base is the number of available digits.
- [[Boolean algebra]] — The logical domain is false/true.

## Used by

- [[Full adder]] — uses carry and borrow as part of its representation or reasoning.

## Recall and next depth

Explain carry and borrow without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** multi-precision arithmetic and architecture-specific conventions.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
