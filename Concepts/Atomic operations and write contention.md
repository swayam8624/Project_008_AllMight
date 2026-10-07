---
tags:
  - concept
  - future
---

# Atomic operations and write contention

**Curriculum depth:** Forward · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

Independently meaningful fields sharing a word can require coordinated read-modify-write and can cause unrelated writers to contend.

An atomic update is indivisible to other participating accesses under its specified memory ordering. Packed independent flags may share one read-modify-write location: writers can overwrite each other without coordination. Contention means accesses compete for ownership or serialization.

## Prerequisites

- [[Bit packing]] — Packing places several constrained values into a carrier.
- [[Machine word]] — A processor's natural integer processing width is often called its word width, but the term is architecture dependent.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain atomic operations and write contention without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** atomics, false sharing, memory ordering, and lock-free updates.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
