---
tags:
  - concept
  - foundation
---

# Hexadecimal

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

Base 16 maps one digit to four bits, preserving visible bit structure compactly.

Hex digits 0–9 and A–F represent values 0–15. A nibble is four bits, so `1010 1101₂` is `AD₁₆`. Group from the right and pad only the left when needed. Human digit order does not determine array packing order.

## Prerequisites

- [[Positional notation]] — A base is the number of available digits.
- [[Bit significance]] — LSB means least significant bit, normally position 0 with weight 1.

## Used by

- [[Floating-point encoding and decoding]] — uses hexadecimal to establish this mechanism's representation or policy.

Connect later chunks here when they use this concept.

## Recall and next depth

Explain hexadecimal without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** hexadecimal floating literals, dumps, addresses, and color/format conventions.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
