---
tags:
  - concept
  - cpp
---

# Relocation versus move

**Curriculum depth:** Introduced · **First seen:** C009 · **Latest development:** C009

## Definition and mechanism

A move leaves the source object alive; relocation ends its source lifetime as part of transfer. Move-construction plus old-object destruction is a relocation pattern, not general permission to memmove nontrivial objects. Proposal/toolchain facilities are not assumed portable in C++23.

## Prerequisites

- [[Move operations and moved-from states]] — Traits, failure policy and relocation must distinguish source survival from source destruction.
- [[Object construction and storage reuse]] — A new destination requires a valid lifetime, independently of available bytes.

## Used by

Later sources will extend this node when their guarantees are developed.

## Recall and next depth

Identify how a self-pointer could break representation relocation. Pending: versioned standard facilities and actual toolchain support. Written coverage does not demonstrate personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#From three repairs to five decisions to zero boilerplate|Teaching mechanism and examples]] · [[Supplementary/Foundations#M009 - Parts 127-147|Exact coverage]] · [[Dependency Map]]
