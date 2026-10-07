---
tags:
  - concept
  - foundation
---

# Fixed-width integer types

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

`<cstdint>` exact-width types express binary layout contracts; `least` and `fast` families have different portability goals.

When supplied, std::uint32_t has exactly 32 bits; uint_least32_t has at least 32; uint_fast32_t is chosen for speed with at least 32. Exact-width typedefs are optional on implementations lacking suitable types. size_t represents object sizes and ptrdiff_t differences.

## Prerequisites

- [[Machine word]] — A processor's natural integer processing width is often called its word width, but the term is architecture dependent.
- [[Byte and octet]] — An octet is exactly eight bits.

## Used by

- [[Bit casting and representation]] — uses fixed-width integer types to establish this mechanism's representation or policy.

- [[Integral promotion]] — uses fixed-width integer types as part of its representation or reasoning.
- [[Serialization]] — uses fixed-width integer types as part of its representation or reasoning.
- [[Guest and host integer domains]] — uses fixed-width integer types as part of its representation or reasoning.

- [[Byte swapping and network order]] — depends on this concept; see its prerequisite explanation.
- [[ABI and object layout]] — depends on this concept; see its prerequisite explanation.

## Recall and next depth

Explain fixed-width integer types without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** availability, ABI layout, byte types, and serialization schemas.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
