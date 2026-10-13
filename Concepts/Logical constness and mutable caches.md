---
tags:
  - concept
  - cpp
---

# Logical constness and mutable caches

**Curriculum depth:** Introduced · **First seen:** C008 · **Latest development:** C008

## Definition and mechanism

Logical constness preserves public meaning while eligible mutable data members may change private state. A cache needs source/cache synchronization and invalidation. Copy a snapshot under protection rather than returning a reference that can outlive the lock. Mutable lambda captures are private closure state, not automatically thread-safe.

## Prerequisites

- [[Const qualification and shallow constness]] — Access-path qualification constrains nested conversions and member writes.
- [[Class access and invariants]] — Internal cache changes must preserve the public invariant.

## Used by

Later sources will deepen this same node; no empty dependent nodes are created.

## Recall and next depth

State the cache invariant and show how Set invalidates it. Pending: versioned publication, atomics and copy/move policy for mutex-bearing classes. Written coverage does not establish learner mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A cache may change bits without changing meaning|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
