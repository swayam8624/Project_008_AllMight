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

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]

## Reconstruction depth

Fragmentation is wasted allocation capacity caused by request shapes and allocator policy. Internal fragmentation is unused space inside an allocated block because of alignment, size classes or rounding. External fragmentation is free memory split into pieces that cannot satisfy a larger request even when total free bytes are sufficient.

A game or graphics workload amplifies the issue because lifetimes are structured: per-frame temporaries, long-lived assets, streaming resources and transient staging data behave differently. Using one general heap for all of them can mix lifetimes and make reuse less predictable.

A useful experiment records requested bytes, committed bytes, peak live bytes, largest free span, allocation count and reuse over time. Frame arenas eliminate external fragmentation inside a frame by resetting wholesale; pools trade generality for fixed-size reuse; free lists require split and coalesce policy.

Fragmentation is not measured by "the process uses a lot of memory" alone. Residency, allocator metadata, page commitment and application-owned slack must be separated before choosing a remedy.

