---
tags:
  - concept
  - foundation
---

# Big endian

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

The lowest address holds the most-significant octet. For 0x12345678, offsets 0–3 hold 12/34/56/78. A decoder repeatedly multiplies its prefix by 256 and adds the next octet. Integer value is independent of host order.

## Prerequisites

- [[Endianness]] — Byte order tells how digit significance maps to sequence positions.
- [[Positional notation]] — Base-256 weighted digits give the codec's numeric meaning.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Derive the base-256 prefix recurrence.

**Pending:** Protocol-specific formats and endian-aware tooling.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
