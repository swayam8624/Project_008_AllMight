---
tags:
  - concept
  - foundation
---

# Huge pages and TLB reach

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

A larger page covers more bytes per translation. With T same-size cached entries, illustrative reach is T×P bytes. One GiB requires 262144 pages at 4 KiB versus 512 at 2 MiB. Larger pages trade flexibility, availability, fragmentation and fault granularity; the counts alone do not prove performance improvement.

## Prerequisites

- [[TLB]] — Larger pages change the amount of memory covered by cached translations.
- [[Allocator fragmentation]] — Coarser allocation granularity introduces space/availability trade-offs.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Calculate page counts without asserting a universal speedup.

**Pending:** Mixed page sizes, workloads, benchmarks and OS policy.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
