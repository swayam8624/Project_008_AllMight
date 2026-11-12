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

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]

## Reconstruction depth

Little-endian byte order stores the least-significant byte first. The 32-bit integer

$$
0x12345678
$$

appears in increasing memory addresses as

$$
78\;56\;34\;12.
$$

That layout is useful only after the interpretation contract is known: the same four bytes could be part of an integer, float, packed header or text. Endianness never supplies the type by itself.

Most contemporary desktop CPUs are little-endian, but that is not permission to encode a durable binary format with raw object bytes. A format needs explicit field widths, byte order, versioning and bounds checks. If a file is defined little-endian, decode each field as little-endian even on a little-endian host; the explicit operation documents the contract and preserves portability.

Tests should use byte arrays whose numeric values are obvious, include unaligned offsets, and verify both read and write paths. For floating point, byte order and IEEE-754 representation are separate assumptions.

