---
tags:
  - concept
  - cpp
---

# Weak observation and ownership cycles

**Curriculum depth:** Introduced · **First seen:** C010 · **Latest development:** C010

## Definition and mechanism

`std::weak_ptr<T>` observes a `shared_ptr` ownership group without increasing its strong count. `lock()` atomically attempts to create a temporary strong owner; the result is empty if the managed object is no longer alive. Replacing one edge of an otherwise all-strong ownership cycle with a weak observation can allow the strong count to reach zero. A check with `expired()` followed later by access is not equivalent to a successful `lock()`.

`enable_shared_from_this` joins the already-established shared ownership group; it must not create a second `shared_ptr` control block from `this`.

## Prerequisites

- [[Exclusive and shared smart-pointer ownership]] — Weak observation belongs to a real shared ownership group.

## Used by

- [[Deferred captures and borrowed views]]
- [[RAII and resource handles]]

## Recall and next depth

Draw a parent/child graph with one weak edge and prove that destruction can proceed. Pending: callback registries, shutdown races, and ownership-group identity.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Weak observation breaks cycles without pretending an object is alive|M010 teaching narrative]] · [[Supplementary/Foundations#M010 - Parts 148-160|Source coverage]]
