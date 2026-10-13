---
tags:
  - concept
  - cpp
---

# Borrow contracts and pool epochs

**Curriculum depth:** Introduced · **First seen:** C008 · **Latest development:** C008

## Definition and mechanism

An epoch versions a borrow-validity interval. A checked borrow compares its saved epoch to the live pool counter before using the cached element. The pool counter must outlive the borrow; counters alone do not supply ownership, synchronization, race freedom or wrap immunity.

## Prerequisites

- [[Borrow invalidation and logical identity]] — Invalidating mutations define the deadlines a saved epoch or deferred borrow must respect.

## Used by

Later sources will deepen this same node; no empty dependent nodes are created.

## Recall and next depth

List every mutation that advances the epoch and identify check-then-use races. Pending: production wrappers, wrap policy and concurrent lifetime pinning. Written coverage does not establish learner mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#When a valid borrow stops naming the intended thing|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
