---
tags:
  - concept
  - foundation
---

# Memory-mapped files

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

A file mapping makes file-backed bytes accessible through a virtual region. Fault handling can supply pages lazily; ownership, access flags, mapping length, and synchronization still matter. A mapped byte buffer is not automatically a set of live aligned C++ objects. File persistence and virtual access are separate mechanisms.

## Prerequisites

- [[Page faults and demand paging]] — Deferred private copying is implemented through a recoverable write fault.
- [[Ownership and non-owning views]] — A mapping or publication pipeline must keep referenced bytes valid.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Separate mapping setup, first-touch loading, typed parsing and persistence.

**Pending:** Platform mapping APIs, sharing modes, truncation and synchronization.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
