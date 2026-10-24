---
tags:
  - concept
  - cpp
---

# Destruction and partial construction

**Curriculum depth:** Developed · **First seen:** C009 · **Latest development:** C010

## Definition and mechanism

The destructor body precedes reverse member/base cleanup. Ordinary constructor failure destroys completed subobjects but not the incomplete whole; failure after a delegating target completed has a distinct cleanup case. RAII binds resource release to an owning object's lifetime.

M010 extends the cleanup invariant to files, locks and native handles, and distinguishes destruction from external resource completion. A destructor can enqueue deferred release when an asynchronous user still holds the resource. Constructor failure remains safe when each acquired duty is immediately represented by a completed owner member.

## Prerequisites

- [[Member order and delegating constructors]] — Reverse cleanup depends on the order and completeness of subobject construction.
- [[Storage duration and object lifetime]] — Cleanup belongs to an occupant's lifetime, not merely a numeric storage address.

## Used by

- [[Assignment and exception guarantees]]
- [[Move operations and moved-from states]]
- [[RAII and resource handles]]

## Recall and next depth

Reconstruct both recorded failure event arrays. Pending: smart owners, polymorphic cleanup and explicit lifetime restart. Written coverage does not demonstrate personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Destruction closes the ownership interval|Teaching mechanism and examples]] · [[Supplementary/Foundations#M009 - Parts 127-147|Exact coverage]] · [[Dependency Map]]
