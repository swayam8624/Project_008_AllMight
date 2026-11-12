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

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]

## Reconstruction depth

The Translation Lookaside Buffer caches recent virtual-page to physical-frame translations. Without a cached translation, the processor performs a page-table walk, which itself requires memory accesses and can stall an otherwise cache-friendly loop.

TLB locality differs from data-cache locality. A traversal can touch one cache line from thousands of separate pages: each useful access is small, but the page working set can exceed TLB capacity. Huge pages increase TLB reach because one translation covers more bytes, although they introduce allocation and fragmentation trade-offs.

When profiling, do not diagnose every unexplained memory stall as a TLB problem. Hardware performance counters can separate cache misses, page walks and TLB misses on supported CPUs. Synthetic experiments should vary stride and working-set pages while keeping useful work controlled.

For engine data, dense arrays often help both cache and TLB locality; pointer-heavy structures can hurt both. NUMA adds another axis: a translation may be cached while the physical page still resides on a remote node.

