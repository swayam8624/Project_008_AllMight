---
tags:
  - concept
  - foundation
---

# Binary conversion

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

Repeated division emits low-to-high binary digits; weighted sums or Horner evaluation decode them.

Euclidean division writes n = 2q + r, where q is the integer quotient and r is 0 or 1. Repeated remainders give bits from least to most significant. Zero must be represented explicitly as `0`. Horner evaluation reads left to right: accumulator = 2×accumulator + next bit.

## Prerequisites

- [[Positional notation]] — A base is the number of available digits.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain binary conversion without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** fractional conversion and arbitrary-precision representation.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
