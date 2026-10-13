---
tags:
  - concept
  - cpp
---

# Rules of Three Five and Zero

**Curriculum depth:** Introduced · **First seen:** C009 · **Latest development:** C009

## Definition and mechanism

Three coordinates destruction/copy construction/copy assignment; Five adds both moves. These are semantic design decisions, not requirements to implement copying. Zero composes members with correct ownership so enclosing classes avoid custom transfer/cleanup boilerplate; domain invariants still need design.

## Prerequisites

- [[Special member generation and traits]] — The actual usable construction expression determines trait-controlled transfer selection.
- [[Assignment and exception guarantees]] — A type's transfer declarations must support its chosen failure transaction.
- [[Ownership and non-owning views]] — The owner/view distinction determines whether copying duplicates resources or descriptors.

## Used by

Later sources will extend this node when their guarantees are developed.

## Recall and next depth

Replace the teaching raw byte owner with a vector and identify remaining invariant obligations. Pending: reusable unique GPU/OS owners and error policies. Written coverage does not demonstrate personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#From three repairs to five decisions to zero boilerplate|Teaching mechanism and examples]] · [[Supplementary/Foundations#M009 - Parts 127-147|Exact coverage]] · [[Dependency Map]]
