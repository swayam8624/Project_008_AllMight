---
tags:
  - concept
  - foundation
---

# Binary16

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

IEEE binary16 has 1/5/10 fields, bias15 and11 normal precision bits. Word3E00 is1.5; maximum finite65504; normal minimum2^-14 and subnormal minimum2^-24.

## Prerequisites

- [[IEEE-754 floating point]] — supplies the representation or validity rule used above.
- [[Rounding to nearest ties to even]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Actual GPU arithmetic and conversion support are not tested by the scalar codec.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]

## Reconstruction depth

IEEE binary16 uses 1 sign bit, 5 exponent bits and 10 stored fraction bits. Normal numbers have an implicit leading one, giving 11 bits of significand precision. The narrow exponent and significand make half precision cheaper to store and move than binary32, but not a drop-in replacement for every intermediate.

Around magnitude 1, adjacent normal values are roughly $2^{-10}$ apart. As magnitude grows, spacing grows with the exponent. Subnormals extend representable values toward zero with reduced precision, while exponent all-ones encodes infinities and NaNs.

A graphics or ML pipeline should distinguish storage precision from accumulation precision. Vertex attributes, textures or weights may be stored as binary16 while dot products and reductions accumulate in binary32. This often preserves bandwidth savings without accepting catastrophic accumulation error.

Tests should include exact endpoints around 1, overflow to infinity according to conversion rules, underflow and subnormals, signed zero and NaN propagation policy. Hardware may flush subnormals or use mixed-precision instructions, so performance behavior requires target-specific evidence.

