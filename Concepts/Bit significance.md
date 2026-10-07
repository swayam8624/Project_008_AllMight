---
tags:
  - concept
  - foundation
---

# Bit significance

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

LSB/MSB describe mathematical weights, not byte order in memory.

LSB means least significant bit, normally position 0 with weight 1. MSB is the highest significant position; in an eight-bit unsigned value it has weight 128. A displayed leftmost bit, first packed array element, and first byte at an address are separate ordering choices.

## Prerequisites

- [[Positional notation]] — A base is the number of available digits.

## Used by

- [[Interpretation contract]] — uses bit significance as part of its representation or reasoning.
- [[Hexadecimal]] — uses bit significance as part of its representation or reasoning.
- [[Bit mask]] — uses bit significance as part of its representation or reasoning.
- [[Shift operations]] — uses bit significance as part of its representation or reasoning.
- [[Endianness]] — uses bit significance as part of its representation or reasoning.
- [[Signed magnitude]] — uses bit significance as part of its representation or reasoning.

## Recall and next depth

Explain bit significance without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** bit numbering conventions and their separation from endianness.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
