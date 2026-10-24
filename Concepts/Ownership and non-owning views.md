---
tags:
  - concept
  - foundation
---

# Ownership and non-owning views

**Curriculum depth:** Developed · **First seen:** C003 · **Latest development:** C010

## Definition and mechanism

By-value copies use member semantics, not recursive deep cloning. Rule-Zero composition delegates ownership to members; raw owners must coordinate transfer and release. Copying a pointer/view/handle is not acquiring ownership.

Copying span/string_view or capturing a view by value copies only a descriptor. An owning string capture supplies independent storage; a view or handle does not automatically pin a deferred resource.

The inline C++ laboratory distinguishes NamedVector::to_array, which copies values into a new array, from ArrayVector::view, which aliases actual array elements without owning them. Neither span nor pointer extends backing lifetime. Ownership and RAII are introduced as next-depth connections, not yet a full exception-safety treatment.

M010 makes the distinction operational: an owner is responsible for release; a borrow only accesses. `std::unique_ptr` expresses exclusive ownership, `std::shared_ptr` retains a shared lifetime, and `std::weak_ptr` observes without extending that lifetime. Smart pointers do not remove the borrow/invalidation rules of raw pointers obtained from them. See [[RAII and resource handles]] and [[Exclusive and shared smart-pointer ownership]].

An owner controls a resource's validity and release; a view merely refers to it. Vector owns its element buffer; span does not copy or prolong that buffer's life. Owner destruction or invalidating reallocation can dangle the span. A reader retaining a span needs stable live backing for every read, not only at construction.

## Prerequisites

- [[Storage duration and object lifetime]] — Access requires an object and backing storage that are still valid.
- [[Dynamic allocation and heap]] — Allocated buffers/blocks supply the storage whose ownership and fragmentation matter.

## Used by

- [[Memory-mapped files]] — uses this mechanism as stated in its prerequisites.
- [[Atomic publication and durability]] — uses this mechanism as stated in its prerequisites.
- [[Deferred captures and borrowed views]]
- [[Copy semantics and resource identity]]
- [[Rules of Three Five and Zero]]
- [[RAII and resource handles]]
- [[Exclusive and shared smart-pointer ownership]]

## Recall and next depth

Explain why returning a view into a local vector fails.

**Pending:** smart owners, shared immutable data and scheduling/resource pinning.

## Source and navigation

[[Supplementary/Foundations#M009 - Parts 127-147|C009 development]]

[[Supplementary/Foundations#M008 - Parts 113-126|C008 development and corrections]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]] · [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Bringing the object model back to bytes|C007 development]]
