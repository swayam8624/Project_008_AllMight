---
tags:
  - concept
  - cpp
---

# Copy semantics and resource identity

**Curriculum depth:** Introduced · **First seen:** C009 · **Latest development:** C009

## Definition and mechanism

Copy construction creates a new object using member/type semantics. Pointers and reference bindings preserve associations; vector-backed pixel values duplicate owned contents; copying handles does not clone resources. Deep copying a graph needs a defined sharing and identity-remapping policy.

## Prerequisites

- [[Object identity and generation handles]] — Copying an identifier differs from cloning the resource or application entity.
- [[Ownership and non-owning views]] — The owner/view distinction determines whether copying duplicates resources or descriptors.

## Used by

- [[Assignment and exception guarantees]]
- [[Move operations and moved-from states]]
- [[Special member generation and traits]]

## Recall and next depth

Choose duplicate, share, remap or reject for four engine types. Pending: graph-clone algorithms and immutable shared ownership. Written coverage does not demonstrate personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Copy a meaning rather than just its bytes|Teaching mechanism and examples]] · [[Supplementary/Foundations#M009 - Parts 127-147|Exact coverage]] · [[Dependency Map]]
