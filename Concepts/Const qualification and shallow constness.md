---
tags:
  - concept
  - cpp
---

# Const qualification and shallow constness

**Curriculum depth:** Introduced · **First seen:** C008 · **Latest development:** C008

## Definition and mechanism

Const restricts modification through an access path; another mutable alias can change a mutable referent. An actually const object cannot be made writable by const_cast. Pointer-member constness is shallow: const span<int> permits integer writes whereas span<const int> does not.

## Prerequisites

- [[References and aliases]] — The initial alias and its referent determine later binding and deduction behavior.

## Used by

- [[Multilevel pointer qualification]]
- [[Ref-qualified accessors]]
- [[Logical constness and mutable caches]]
- [[Parameter passing and sink contracts]]

## Recall and next depth

Read all four pointer-qualification combinations aloud. Pending: atomics, volatile hardware access and immutable shared-state design. Written coverage does not establish learner mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Read-only is a path, not an ownership or synchronization promise|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
