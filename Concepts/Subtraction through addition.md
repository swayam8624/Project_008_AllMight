---
tags:
  - concept
  - foundation
---

# Subtraction through addition

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

$A-B=A+\sim B+1$, allowing the same adder to implement both operations; some ISAs fold borrow state into carry-in.

At N-bit residue width, A−B equals A+NOTₙ(B)+1. Binary 6502-style SBC computes A−B−(1−C), equivalently A+NOT₈(B)+C. C=1 means no incoming borrow; output carry=1 means no outgoing borrow.

## Prerequisites

- [[Two's complement]] — For raw unsigned value u in N bits, signed decoding is u when u<2ᴺ⁻¹, otherwise u−2ᴺ.
- [[Full adder]] — A full adder adds A, B, and carry-in.

## Used by

- [[Binary long division]] — uses subtraction through addition as part of its representation or reasoning.

## Recall and next depth

Explain subtraction through addition without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** ISA flag conventions and multi-word subtraction.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
