---
tags:
  - concept
  - foundation
---

# One's complement

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

Negation inverts every bit, leaving two zeros and requiring end-around carry for ordinary addition.

Negative x is the N-bit complement of positive x. Eight-bit −5 is `11111010`; zeros are all zeros and all ones. Adding 5 and encoded −3 gives a carry plus low value 1; end-around carry adds that carry back to yield 2.

## Prerequisites

- [[Bitwise NOT]] — Complement must specify width: eight-bit complement of `0x0F` is `0xF0`, while sixteen-bit complement is `0xFFF0`.
- [[Signed magnitude]] — The high bit holds sign and the remaining bits hold magnitude.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain one's complement without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** Internet checksums and historical machines.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
