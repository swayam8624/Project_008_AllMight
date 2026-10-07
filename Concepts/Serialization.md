---
tags:
  - concept
  - foundation
---

# Serialization

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C004

## Definition and mechanism

Serialization maps semantic fields to a specified byte sequence; decoding validates and reconstructs them. Width, order, version, bounds, and padding policy belong to that contract. A U8/U32-LE/U64-LE header occupies 13 bytes at offsets 0/1/5, independently of a native record's possible 16-byte ABI layout. Packing removes some holes, not host dependencies. Publication and durability are subsequent operations.

M004 extension: a binary32 representation word such as 3F800000 can be serialized as little-endian bytes 00 00 80 3F. Raw-bit preservation retains signed-zero and NaN-payload differences; canonicalization deliberately merges them. Numeric equality does not imply byte identity. This is a format policy, not permission to change existing files.

## Prerequisites

- [[Interpretation contract]] — Representation, type, and lifetime must identify what object exists.
- [[Fixed-width integer types]] — The representation width fixes byte count and valid shift range.
- [[Endianness]] — Byte order tells how digit significance maps to sequence positions.

## Used by

- [[Floating-point canonicalization]] — uses serialization to establish this mechanism's representation or policy.

- [[Atomic publication and durability]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Explain why packed/native memory dumps do not establish a portable schema.

**Pending:** Version migration, malformed-input protocols, other floating formats and cross-platform special-value transport.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|C004 representation policy]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
