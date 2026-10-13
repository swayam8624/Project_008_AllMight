---
tags:
  - concept
  - cpp
---

# Borrow invalidation and logical identity

**Curriculum depth:** Introduced · **First seen:** C008 · **Latest development:** C008

## Definition and mechanism

A borrow relies on an external lifetime/mutation contract. Vector reallocation invalidates element paths; destruction ends a referent's lifetime. Move-assigning a final dense-pool element into a live earlier slot may preserve that C++ object while changing its logical entity association.

## Prerequisites

- [[Pointer arithmetic and provenance]] — Storage relationships, bounds and lifetime distinguish valid access from a non-null value.
- [[Object identity and generation handles]] — Live C++ objects and application entities can diverge when pool slots are repurposed.

## Used by

- [[Borrow contracts and pool epochs]]
- [[Deferred captures and borrowed views]]

## Recall and next depth

Trace owner11 becoming owner33 without first-slot destruction. Pending: per-container invalidation guarantees and stable-storage designs. Written coverage does not establish learner mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#When a valid borrow stops naming the intended thing|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
