---
tags:
  - concept
  - foundation
---

# Binary long division

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

Shift dividend bits into a remainder, compare/subtract the divisor, and emit quotient bits while maintaining $D=Qd+R$.

At each dividend bit, form remainder=2×remainder+bit. If it reaches the divisor, subtract and emit quotient bit 1; otherwise emit 0. For 13/3, quotient 4 and remainder 1 satisfy 13=4×3+1. A wider temporary may be needed for shifted remainder.

## Prerequisites

- [[Shift operations]] — Logical right shift fills high positions with zero; arithmetic right shift replicates the sign bit.
- [[Subtraction through addition]] — At N-bit residue width, A−B equals A+NOTₙ(B)+1.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain binary long division without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** restoring/non-restoring division, signed rounding, reciprocal methods, and hardware latency.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
