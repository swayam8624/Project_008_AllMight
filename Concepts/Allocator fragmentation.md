---
tags:
  - concept
  - foundation
---

# Allocator fragmentation

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

Internal fragmentation is unused space inside an allocated block, e.g. 37 bytes requested in a 48-byte class leaves 11. External fragmentation is free space split into holes incapable of satisfying one contiguous request, e.g. three separated 20-byte holes cannot supply one 60-byte block. Metadata overhead is an additional cost.

## Prerequisites

- [[Dynamic allocation and heap]] — Allocated buffers/blocks supply the storage whose ownership and fragmentation matter.

## Used by

- [[Huge pages and TLB reach]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Distinguish slack, metadata, and separated holes.

**Pending:** Coalescing, size classes, virtual commitment and measured overhead.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
