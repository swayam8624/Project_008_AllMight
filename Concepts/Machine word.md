---
tags:
  - concept
  - foundation
---

# Machine word

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C002

## Definition and mechanism

A context-dependent natural processing width, not a portable fixed-size unit.

A processor's natural integer processing width is often called its word width, but the term is architecture dependent. A 64-bit CPU still supports smaller integer operations and wider vector operations. State exact widths in formats instead of relying on the word label.

## Prerequisites

- [[Byte and octet]] — An octet is exactly eight bits.

## Used by

- [[Bitwise NOT]] — uses machine word as part of its representation or reasoning.
- [[Shift operations]] — uses machine word as part of its representation or reasoning.
- [[Unsigned modular arithmetic]] — uses machine word as part of its representation or reasoning.
- [[Atomic operations and write contention]] — uses machine word as part of its representation or reasoning.
- [[Compiler IR and machine instructions]] — uses machine word as part of its representation or reasoning.
- [[SIMD and GPU data layouts]] — uses machine word as part of its representation or reasoning.
- [[Fixed-width integer types]] — uses machine word as part of its representation or reasoning.

## Recall and next depth

Explain machine word without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** registers, instruction width, SIMD lanes, and ABI data models.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 development]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
