---
tags:
  - concept
  - foundation
---

# SIMD and GPU data layouts

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C003

## Definition and mechanism

Wider words, lanes, buffers, pixels, and vertex formats reuse exact-width representation and packing principles.

SIMD applies an operation across multiple data lanes; GPU buffers and formats specify how values are arranged for parallel access. Packing can reduce traffic but adds decoding and alignment constraints. Lane layout, alignment, and access grouping require later treatment.

## Prerequisites

- [[Machine word]] — A processor's natural integer processing width is often called its word width, but the term is architecture dependent.
- [[Alignment and padding]] — Alignment is a required address multiple for an object.

## Used by

Connect later chunks here when they use this concept.

## M003 development

Alignment of vector loads and buffer offsets is a separate requirement from endian order. [[Packed structures and misaligned access]] may save space but introduce boundary crossings; [[Field reordering and AoS versus SoA]] can improve fieldwise access. CPU ABI layout is not automatically a GPU buffer schema.

## Recall and next depth

Explain simd and gpu data layouts without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** vector lanes, alignment, coalescing, and resource formats.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 development]]
