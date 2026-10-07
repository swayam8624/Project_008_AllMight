---
tags:
  - concept
  - foundation
---

# Field reordering and AoS versus SoA

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

Reordering fields can reduce alignment holes: conventional 1/8/1/4 members shrink 24→16 bytes when arranged 8/4/1/1. AoS (array of structures) interleaves full records; SoA (structure of arrays) separates each field into its own array. Smaller records do not prove faster code; hot fields and access patterns matter, and layout changes may break ABI.

## Prerequisites

- [[Internal and tail padding]] — The position and amount of unused layout space determine reordering/packing effects.
- [[Cache lines and memory bandwidth]] — Field adjacency and stride affect useful bytes fetched per access.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Calculate footprint savings, then ask which fields a loop actually reads.

**Pending:** Measured AoS/SoA/AoSoA trade-offs, ECS and SIMD.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
