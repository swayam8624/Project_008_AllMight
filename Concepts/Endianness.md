---
tags:
  - concept
  - foundation
---

# Endianness

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C003

## Definition and mechanism

Byte order maps numeric significance to ascending addresses without changing the value. Little endian emits the low octet first; big endian emits the high octet first. The 32-bit value 0x12345678 becomes 78/56/34/12 or 12/34/56/78 respectively. It does not specify intra-byte bit order. Explicit unsigned shifts/masks define an external representation independent of native host order.

## Prerequisites

- [[Byte and octet]] — Byte units define both storage offsets and significance digits.
- [[Bit significance]] — Distinguish byte significance from where it is stored.

## Used by

- [[Memory dump interpretation]] — uses this mechanism to define its representation, error or validation contract.

- [[Little endian]] — uses this mechanism as stated in its prerequisites.
- [[Big endian]] — uses this mechanism as stated in its prerequisites.
- [[Byte swapping and network order]] — uses this mechanism as stated in its prerequisites.
- [[Serialization]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Derive four bytes, invert each matching codec, and explain the mismatched decode.

**Pending:** Mixed endian representations and protocol-specific rules.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
