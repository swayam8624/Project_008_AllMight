---
tags:
  - concept
  - foundation
---

# Dynamic allocation and heap

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

An allocator manages variable-lifetime blocks from larger memory regions; new/free-store usage is commonly called heap allocation. An allocated object's later latency depends on locality, not the heap label. Vector metadata may be automatic/static while its element buffer is separately dynamic. Over-aligned types need suitable allocation support.

## Prerequisites

- [[Storage duration and object lifetime]] — Access requires an object and backing storage that are still valid.
- [[Alignment and padding]] — Address divisibility determines legal placement and layout holes.

## Used by

- [[Allocator fragmentation]] — uses this mechanism as stated in its prerequisites.
- [[Ownership and non-owning views]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Separate allocation work, element buffer location, and access cost.

**Pending:** Arenas, slabs, size classes, synchronization and resource allocators.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
