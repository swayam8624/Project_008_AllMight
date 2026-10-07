---
tags:
  - concept
  - foundation
---

# Atomic publication and durability

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

Publishing a prepared temporary file by replacement concerns observers of the filesystem namespace. Crash durability concerns whether contents and namespace updates survive failures; it needs a specified platform/filesystem protocol. Neither follows from building a vector of bytes. The supplied AtomicFile excerpt is an unverified repository claim, not a tested durability guarantee.

## Prerequisites

- [[Serialization]] — Publication persists already-specified bytes, rather than defining their format.
- [[Ownership and non-owning views]] — A mapping or publication pipeline must keep referenced bytes valid.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Name the observers and failure model for an atomic guarantee.

**Pending:** Platform replacement semantics, flush ordering, recovery and cross-volume limits.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
