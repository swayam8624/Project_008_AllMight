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

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]

## Reconstruction depth

A virtually contiguous array occupies one uninterrupted range of virtual addresses. The page table may map those virtual pages to physical frames scattered through RAM. Most user-space code only requires virtual contiguity because ordinary loads and stores are translated by the MMU before reaching memory.

Physical contiguity matters at different boundaries: some DMA devices, huge-page mappings, kernels or fixed hardware buffers may impose stronger requirements. Modern IOMMUs can themselves provide device-visible address translation, reducing the need for physically contiguous host memory in some designs.

This explains why a large dynamic allocation can succeed even when free physical RAM is fragmented into many regions: the operating system can assemble the virtual range from separate frames and may not commit all of them until faults occur.

For performance, virtual contiguity still does not guarantee identical locality. Page placement can span NUMA nodes, and TLB reach depends on page size and access footprint. Therefore contiguous storage is a statement about address iteration and element layout, not a complete claim about physical placement.

