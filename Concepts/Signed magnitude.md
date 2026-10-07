---
tags:
  - concept
  - foundation
---

# Signed magnitude

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

The high bit selects sign while remaining bits encode magnitude; this is intuitive but has positive and negative zero and needs sign-aware arithmetic.

The high bit holds sign and the remaining bits hold magnitude. Eight-bit +5 is `00000101`, −5 is `10000101`, and `00000000` and `10000000` both denote zero. Mixed signs require magnitude comparison and subtraction rather than ordinary raw-bit addition.

## Prerequisites

- [[Positional notation]] — A base is the number of available digits.
- [[Bit significance]] — LSB means least significant bit, normally position 0 with weight 1.

## Used by

- [[One's complement]] — uses signed magnitude as part of its representation or reasoning.

## Recall and next depth

Explain signed magnitude without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** floating-point sign fields and non-integer signed encodings.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
