---
tags:
  - concept
  - cpp
---

# Null pointers and optional access

**Curriculum depth:** Introduced · **First seen:** C008 · **Latest development:** C008

## Definition and mechanism

A null pointer designates no object/function; nullptr_t converts to appropriate null pointer values without choosing an integer overload. A non-null pointer is not a lifetime or bounds proof. Deleting a typed null object pointer is valid, but bare delete nullptr is ill-formed.

## Prerequisites

- [[Pointer arithmetic and provenance]] — Storage relationships, bounds and lifetime distinguish valid access from a non-null value.

## Used by

Later sources will deepen this same node; no empty dependent nodes are created.

## Recall and next depth

Explain absence, stale access and one-past access separately. Pending: optional/reference wrapper APIs and owning-pointer policies. Written coverage does not establish learner mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A pointer carries an access contract|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
