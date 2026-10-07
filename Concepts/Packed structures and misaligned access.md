---
tags:
  - concept
  - foundation
---

# Packed structures and misaligned access

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

Compiler packing extensions can weaken aggregate/member alignment and remove holes. A uint32 member at offset one may be misaligned. Direct extension-aware member access may receive special code generation; extracting an ordinary typed pointer can lose that treatment. Hardware tolerance does not establish legal C++ access. Explicit codecs avoid alignment, lifetime, and host-layout assumptions.

## Prerequisites

- [[Alignment and padding]] — Address divisibility determines legal placement and layout holes.
- [[Internal and tail padding]] — The position and amount of unused layout space determine reordering/packing effects.
- [[Undefined behavior]] — Hardware completion does not prove a language-level operation legal.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Distinguish packing, endian normalization, and legal typed access.

**Pending:** Instruction-specific costs and mapped-file parsing.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
