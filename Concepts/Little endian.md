---
tags:
  - concept
  - foundation
---

# Little endian

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

The lowest address holds the least-significant octet. For 0x12345678, offsets 0–3 hold 78/56/34/12. Writer byte i selects bits 8i through 8i+7; a matching reader restores the same weights. This is byte order, not bit reversal.

## Prerequisites

- [[Endianness]] — Byte order tells how digit significance maps to sequence positions.
- [[Shift operations]] — Shifts relocate a selected byte or address field to its significance position.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Write and decode four bytes, including leading zero bytes.

**Pending:** Mixed-endian cases and protocol integration.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
