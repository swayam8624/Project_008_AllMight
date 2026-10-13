---
tags:
  - concept
  - cpp
---

# Noexcept and transfer selection

**Curriculum depth:** Introduced · **First seen:** C009 · **Latest development:** C009

## Definition and mechanism

noexcept is an escaping-exception contract; violation terminates. move_if_noexcept selects const T& when a throwing move and usable copy make rollback safer, otherwise T&&. It casts rather than transfers, and non-copyable throwing moves may weaken operation-specific guarantees.

## Prerequisites

- [[Move operations and moved-from states]] — Traits, failure policy and relocation must distinguish source survival from source destruction.
- [[Assignment and exception guarantees]] — A type's transfer declarations must support its chosen failure transaction.
- [[Special member generation and traits]] — The actual usable construction expression determines trait-controlled transfer selection.

## Used by

Later sources will extend this node when their guarantees are developed.

## Recall and next depth

Derive both tested helper result types and the rollback limitation. Pending: exact per-container exceptional effects and allocator propagation. Written coverage does not demonstrate personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Failure guarantees shape the transfer algorithm|Teaching mechanism and examples]] · [[Supplementary/Foundations#M009 - Parts 127-147|Exact coverage]] · [[Dependency Map]]
