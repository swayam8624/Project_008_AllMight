---
tags:
  - concept
  - cpp
---

# Deferred captures and borrowed views

**Curriculum depth:** Introduced · **First seen:** C008 · **Latest development:** C008

## Definition and mechanism

A span or string_view describes data without owning it. Copying a view copies its descriptor, not its backing storage. Reference captures and captured this pointers can outlive their referents; copying an owning string into the closure creates a separate owned snapshot.

## Prerequisites

- [[Ownership and non-owning views]] — Copying a descriptor does not copy or own its data.
- [[Temporary lifetime extension]] — Temporary deadlines constrain returned paths and deferred access.
- [[Borrow invalidation and logical identity]] — Invalidating mutations define the deadlines a saved epoch or deferred borrow must respect.

## Used by

Later sources will deepen this same node; no empty dependent nodes are created.

## Recall and next depth

Contrast capturing string versus string_view by value. Pending: scheduling, cancellation and ownership-pinning policies. Written coverage does not establish learner mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Borrowed views and deferred work need an owner story|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
