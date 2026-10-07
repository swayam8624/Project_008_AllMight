---
tags:
  - concept
  - foundation
---

# Shift-and-add multiplication

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

Each set multiplier bit selects a shifted multiplicand partial product; an exact product of two $N$-bit values may need $2N$ bits.

Multiplier bit i selects multiplicand shifted i positions. 13×11 uses 11=`1011₂`, hence 13+26+104=143. Keep the shifting multiplicand in the widened type; shifting a uint16_t variable repeatedly discards high partial-product bits even if the accumulator is uint32_t.

## Prerequisites

- [[Shift operations]] — Logical right shift fills high positions with zero; arithmetic right shift replicates the sign bit.
- [[Full adder]] — A full adder adds A, B, and carry-in.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain shift-and-add multiplication without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** Booth recoding, compressor trees, pipelining, and signed multiplication.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
