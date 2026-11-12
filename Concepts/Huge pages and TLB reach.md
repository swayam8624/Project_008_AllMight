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

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]

## Reach, fragmentation, and measurement

TLB reach is approximately the number of cached translations multiplied by the page size. If a TLB can hold $N$ translations, switching from 4 KiB pages to 2 MiB huge pages can increase the bytes covered by those translations by a factor of 512.

That does not make huge pages universally faster. They can increase internal fragmentation, make allocation/compaction harder, change NUMA placement behavior and waste memory when only small portions of each huge page are touched. Transparent Huge Pages may also promote or split mappings asynchronously, so an application should not infer actual page size from its allocation API alone.

The right benchmark holds the useful computation fixed while varying working-set size and access stride, then observes TLB/page-walk counters where available. A speedup from huge pages should correlate with translation pressure rather than simply with a different allocator or cache footprint.

For engines, huge pages are most plausible for stable, large, densely accessed arenas—not as a default for every allocation.

