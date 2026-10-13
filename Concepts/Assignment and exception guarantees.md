---
tags:
  - concept
  - cpp
---

# Assignment and exception guarantees

**Curriculum depth:** Introduced · **First seen:** C009 · **Latest development:** C009

## Definition and mechanism

Assignment replaces an already-live target. Prepare a complete replacement before a non-throwing commit to preserve the old target on failure; copy-and-swap then releases old state through temporary destruction. Strong, basic and non-throwing guarantees are separate contracts.

## Prerequisites

- [[Copy semantics and resource identity]] — Assignment and moving must implement a defined semantic value rather than a raw pointer copy.
- [[Constructors and invariant establishment]] — Initialization form and structural order determine how the constructor establishes members.
- [[Destruction and partial construction]] — Transfer algorithms need a safe source/temporary cleanup path on success and failure.

## Used by

- [[Noexcept and transfer selection]]
- [[Rules of Three Five and Zero]]

## Recall and next depth

Inject allocation failure before commit and predict both owners. Pending: capacity-reusing assignment and subsystem-wide rollback. Written coverage does not demonstrate personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Replacing a value is not creating an object|Teaching mechanism and examples]] · [[Supplementary/Foundations#M009 - Parts 127-147|Exact coverage]] · [[Dependency Map]]
