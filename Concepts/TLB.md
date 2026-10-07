---
tags:
  - concept
  - foundation
---

# TLB

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

The translation lookaside buffer caches recent address translations and relevant access metadata, not the bytes being loaded. A miss may trigger a valid page-table walk without OS fault handling. A TLB hit can coexist with a data-cache miss. Context and permissions matter; it is not just a global VPN-only dictionary.

## Prerequisites

- [[Page tables and MMU]] — Cached or faulting translation starts from the mapping/protection mechanism.

## Used by

- [[Huge pages and TLB reach]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Distinguish a translation miss, cache miss, and page fault.

**Pending:** ASIDs/PCIDs, shootdowns, hierarchy, associativity, and measured reach.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
