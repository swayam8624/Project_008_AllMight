---
tags:
  - concept
  - foundation
---

# Minimum signed value

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

Two's-complement range has one extra negative value, so `INT_MIN` cannot be negated or represented by same-type absolute value.

N-bit two's complement spans −2ᴺ⁻¹ through 2ᴺ⁻¹−1. Eight-bit minimum is −128, whose positive counterpart 128 does not fit. Negating int minimum or dividing it by −1 overflows the operation type. Narrow int8_t operands may promote first, so the original storage width alone does not establish UB.

## Prerequisites

- [[Two's complement]] — For raw unsigned value u in N bits, signed decoding is u when u<2ᴺ⁻¹, otherwise u−2ᴺ.

## Used by

- [[Signed integer overflow]] — uses minimum signed value as part of its representation or reasoning.

## Recall and next depth

Explain minimum signed value without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** Apply and reconstruct this concept independently.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
