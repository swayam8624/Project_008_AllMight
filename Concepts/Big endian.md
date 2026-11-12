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

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]

## Reconstruction depth

Big-endian byte order stores the most-significant byte of a multibyte integer at the lowest address. For the 32-bit value

$$
0x12345678,
$$

the four bytes in increasing address order are

$$
12\;34\;56\;78.
$$

Endianness describes byte ordering, not bit ordering inside a byte and not the abstract value of the integer. Arithmetic on a correctly loaded integer is independent of whether its bytes arrived in big- or little-endian form.

The engineering boundary is serialization. A file or network protocol must declare byte order explicitly, because dumping a host struct makes the external format depend on machine layout, padding and host endianness. Network protocols traditionally use big-endian "network byte order", but a custom format may choose either convention as long as decoding is explicit.

A reliable parser reads bytes, validates length, reconstructs the integer with shifts or standardized byte-swap facilities, and tests known byte sequences. The failure to avoid is double-swapping: once bytes have been converted into host numeric value, downstream code should not keep carrying an ambiguous "network-order integer" type unless the API makes that distinction explicit.

