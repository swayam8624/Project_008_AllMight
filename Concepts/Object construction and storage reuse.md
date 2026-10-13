---
tags:
  - concept
  - cpp
---

# Object construction and storage reuse

**Curriculum depth:** Developed · **First seen:** C007 · **Latest development:** C009

## Definition and mechanism

Initialization creates an occupant whereas assignment replaces an existing occupant's state. Ordinary constructor failure and post-target delegation failure have different complete-object cleanup behavior.

Storage needs sufficient size/alignment before object initialization can complete and start an ordinary lifetime. construct_at and destroy_at separate object life from backing storage. A cast alone does not construct arbitrary types. Some specified operations implicitly create implicit-lifetime objects; replacement and launder have additional preconditions.

## Prerequisites

- [[Storage duration and object lifetime]] — Backing-storage duration must be distinguished from the occupant's start, destruction and replacement.

## Used by

- [[Object identity and generation handles]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.
- [[Return values and copy elision]]
- [[Constructors and invariant establishment]]
- [[Relocation versus move]]

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** explicit lifetime restart, smart ownership and allocator protocols.

## Source and navigation

[[Supplementary/Foundations#M009 - Parts 127-147|C009 development]]

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Storage is a place; an object is its occupant|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
