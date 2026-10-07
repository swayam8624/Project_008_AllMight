---
tags:
  - concept
  - foundation
---

# Copy-on-write

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

COW initially shares physical contents while deferring private copies. A permitted write to a COW-protected mapping faults; OS allocates/copies a frame, updates the writer's mapping, and retries. Other mappings retain old contents. Not every read-only page is COW, so not every write fault is recoverable.

## Prerequisites

- [[Page faults and demand paging]] — Deferred private copying is implemented through a recoverable write fault.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Show frame identity before and after one process writes.

**Pending:** Fork behavior, mapping flags, reference tracking and costs.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
