---
tags:
  - concept
  - foundation
---

# Fixed-width integer types

**Curriculum depth:** Developed · **First seen:** C002 · **Latest development:** C007

## Definition and mechanism

int need not be32 bits; sizeof(char)==1 measures a C++ byte and CHAR_BIT is at least8. Plain char is distinct from signed/unsigned char and its signedness is implementation-defined. size_t represents object sizes, not a stable serialized field. Query numeric_limits for digits, lowest, max and floating capabilities. A container's size_type is not universally required to be exactly size_t.

`<cstdint>` exact-width types express binary layout contracts; `least` and `fast` families have different portability goals.

When supplied, std::uint32_t has exactly 32 bits; uint_least32_t has at least 32; uint_fast32_t is chosen for speed with at least 32. Exact-width typedefs are optional on implementations lacking suitable types. size_t represents object sizes and ptrdiff_t differences.

## Prerequisites

- [[Machine word]] — A processor's natural integer processing width is often called its word width, but the term is architecture dependent.
- [[Byte and octet]] — An octet is exactly eight bits.

## Used by

- [[Scoped enums and validated domains]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

- [[Bit casting and representation]] — uses fixed-width integer types to establish this mechanism's representation or policy.

- [[Integral promotion]] — uses fixed-width integer types as part of its representation or reasoning.
- [[Serialization]] — uses fixed-width integer types as part of its representation or reasoning.
- [[Guest and host integer domains]] — uses fixed-width integer types as part of its representation or reasoning.

- [[Byte swapping and network order]] — depends on this concept; see its prerequisite explanation.
- [[ABI and object layout]] — depends on this concept; see its prerequisite explanation.

## Recall and next depth

Explain fixed-width integer types without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** Full lifetime replacement, ownership/concurrency policies and cross-target ABI experiments; the opening examples do not establish those later guarantees.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]] · [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Choose a type for its promise, not its familiar size|C007 development]]
