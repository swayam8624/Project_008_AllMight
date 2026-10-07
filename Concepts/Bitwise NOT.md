---
tags:
  - concept
  - foundation
---

# Bitwise NOT

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C002

## Definition and mechanism

Complements every lane within an explicit representation width.

Complement must specify width: eight-bit complement of `0x0F` is `0xF0`, while sixteen-bit complement is `0xFFF0`. C++ first promotes narrow operands, so `~uint8_t{0}` usually has type int and value −1. Cast or mask intentionally when retaining eight bits.

## Prerequisites

- [[Boolean algebra]] — The logical domain is false/true.
- [[Machine word]] — A processor's natural integer processing width is often called its word width, but the term is architecture dependent.

## Used by

- [[De Morgan's laws]] — uses bitwise not as part of its representation or reasoning.
- [[Two's complement]] — uses bitwise not as part of its representation or reasoning.
- [[One's complement]] — uses bitwise not as part of its representation or reasoning.

## Recall and next depth

Explain bitwise not without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** promotions, signed representations, and masked complements.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 development]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
