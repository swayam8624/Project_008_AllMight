---
tags:
  - concept
  - foundation
---

# Ownership and non-owning views

**Curriculum depth:** Developed · **First seen:** C003 · **Latest development:** C007

## Definition and mechanism

The inline C++ laboratory distinguishes NamedVector::to_array, which copies values into a new array, from ArrayVector::view, which aliases actual array elements without owning them. Neither span nor pointer extends backing lifetime. Ownership and RAII are introduced as next-depth connections, not yet a full exception-safety treatment.

An owner controls a resource's validity and release; a view merely refers to it. Vector owns its element buffer; span does not copy or prolong that buffer's life. Owner destruction or invalidating reallocation can dangle the span. A reader retaining a span needs stable live backing for every read, not only at construction.

## Prerequisites

- [[Storage duration and object lifetime]] — Access requires an object and backing storage that are still valid.
- [[Dynamic allocation and heap]] — Allocated buffers/blocks supply the storage whose ownership and fragmentation matter.

## Used by

- [[Memory-mapped files]] — uses this mechanism as stated in its prerequisites.
- [[Atomic publication and durability]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Explain why returning a view into a local vector fails.

**Pending:** Full lifetime replacement, ownership/concurrency policies and cross-target ABI experiments; the opening examples do not establish those later guarantees.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]] · [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Bringing the object model back to bytes|C007 development]]
