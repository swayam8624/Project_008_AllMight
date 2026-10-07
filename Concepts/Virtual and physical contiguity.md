---
tags:
  - concept
  - foundation
---

# Virtual and physical contiguity

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

Consecutive virtual addresses can map nonadjacent physical frames. Vector's contiguous elements guarantee adjacent object storage in the process model, not one contiguous physical allocation. File offsets and storage-device placement are separate adjacency notions. Contiguity must always name the space being discussed.

## Prerequisites

- [[Pages and page offsets]] — Page identity is translated while within-page position is preserved.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Map three adjacent virtual pages to frames 912/17/404.

**Pending:** DMA, physical allocation, IOMMU and scatter/gather.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
