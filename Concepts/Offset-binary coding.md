---
tags:
  - concept
  - foundation
---

# Offset-binary coding

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

A bias turns signed levels into unsigned codes. For the source-shaped INT4 convention c=q+8, -3 becomes nibble0101, not the two's-complement nibble1101.

## Prerequisites

- [[Unsigned modular arithmetic]] — supplies the representation or validity rule used above.
- [[Interpretation contract]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Do not assume an INT4 label defines offset versus two's-complement storage.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]

## Ordering and bias

Offset-binary represents signed values by adding a fixed bias before storing an unsigned code:

$$q=x+B.$$

With an $n$-bit field and the common midpoint bias $B=2^{n-1}$, the smallest signed value maps near code zero and the largest maps near the all-ones code. Unlike two's complement, the unsigned lexical order of codes then matches the numeric order of represented values. That property is useful in some exponent fields, ADC outputs, sortable binary formats and hardware interfaces.

The bias is part of the format contract. Decoding with the wrong bias shifts every value while still producing plausible numbers. The IEEE-754 exponent field is related but reserves all-zero and all-one exponent codes for subnormals/zero and infinities/NaNs, so it is not simply an unrestricted offset-binary integer.

Tests should check both endpoints, the code corresponding to numeric zero, monotonic ordering, and rejection or handling of reserved codes when the surrounding format defines them.

