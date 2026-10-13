---
tags:
  - concept
  - cpp
---

# Move operations and moved-from states

**Curriculum depth:** Introduced · **First seen:** C009 · **Latest development:** C009

## Definition and mechanism

Move construction creates a destination; move assignment also handles a prior target resource. Ownership transfer must repair the surviving source's documented state. Valid-but-unspecified is not destroyed/uninitialized, and custom types must maintain their own invariants.

## Prerequisites

- [[Move casts and forwarding references]] — An xvalue cast permits selection of a consuming operation without performing it.
- [[Copy semantics and resource identity]] — Assignment and moving must implement a defined semantic value rather than a raw pointer copy.
- [[Destruction and partial construction]] — Transfer algorithms need a safe source/temporary cleanup path on success and failure.

## Used by

- [[Special member generation and traits]]
- [[Noexcept and transfer selection]]
- [[Relocation versus move]]

## Recall and next depth

Move a raw byte owner into an existing owner and identify every release. Pending: allocator-aware moves and cross-member domain invariants. Written coverage does not demonstrate personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A move transfers responsibility while the source survives|Teaching mechanism and examples]] · [[Supplementary/Foundations#M009 - Parts 127-147|Exact coverage]] · [[Dependency Map]]
