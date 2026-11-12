---
tags:
  - concept
  - foundation
---

# Bfloat16

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

BF16 has1/8/7 fields, bias127 and8 normal precision bits. It keeps broad exponent range at coarse spacing2^-7 above1. A nearest-even conversion must handle NaNs separately from finite rounding.

## Prerequisites

- [[IEEE-754 floating point]] — supplies the representation or validity rule used above.
- [[Rounding to nearest ties to even]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Target flushing, payload conventions and accumulation instructions require independent checks.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]

## Range is preserved more than precision

Bfloat16 keeps the 8-bit exponent width of binary32 but stores only 7 explicit fraction bits. A normal value therefore has 8 bits of significand precision including the hidden leading one. Compared with IEEE binary16, bfloat16 sacrifices substantial local precision to retain a binary32-like exponent range.

That trade is useful for ML tensors whose dynamic range matters more than fine per-value precision. It is poor for algorithms where small relative changes accumulate over many steps. Common mixed-precision kernels store operands in bfloat16 but accumulate products in binary32.

Conversion is not merely truncation if the implementation promises round-to-nearest behavior. Correct rounding must account for discarded bits and tie policy; NaN payloads and infinities need explicit handling. Tests should inspect bit-level round trips, halfway cases, overflow/underflow, NaNs, and error growth in dot products accumulated in bfloat16 versus binary32.

