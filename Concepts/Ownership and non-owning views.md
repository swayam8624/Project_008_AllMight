---
tags:
  - concept
  - foundation
---

# Ownership and non-owning views

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

An owner controls a resource's validity and release; a view merely refers to it. Vector owns its element buffer; span does not copy or prolong that buffer's life. Owner destruction or invalidating reallocation can dangle the span. A reader retaining a span needs stable live backing for every read, not only at construction.

## Prerequisites

- [[Storage duration and object lifetime]] — Access requires an object and backing storage that are still valid.
- [[Dynamic allocation and heap]] — Allocated buffers/blocks supply the storage whose ownership and fragmentation matter.

## Used by

- [[Memory-mapped files]] — uses this mechanism as stated in its prerequisites.
- [[Atomic publication and durability]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Explain why returning a view into a local vector fails.

**Pending:** Borrowed ranges, iterator invalidation and explicit lifetime contracts.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
