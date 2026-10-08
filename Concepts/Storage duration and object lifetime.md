---
tags:
  - concept
  - foundation
---

# Storage duration and object lifetime

**Curriculum depth:** Developed · **First seen:** C003 · **Latest development:** C007

## Definition and mechanism

Construction and destruction can delimit an occupant while backing storage remains. The inline C++ lab uses construct_at/destroy_at in an alignas byte buffer and new/delete as the combined allocation/construction and destruction/release operations. The pointer object does not own or extend pointee life. Implicit object creation occurs only for specified operations and suitable types; a reinterpret_cast is not a universal construction operation.

Storage duration specifies how long storage is potentially available: automatic, static, thread, or dynamic. Object lifetime describes when a particular initialized object exists within it. Old bytes can remain after lifetime ends without authorizing access. const restricts mutation, not physical placement. Language categories do not dictate a universal segment map.

## Prerequisites

- [[Interpretation contract]] — Representation, type, and lifetime must identify what object exists.

## Used by

- [[Thread-local storage]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

- [[Object construction and storage reuse]] — separates an occupant's construction from backing storage availability.

- [[Pointer arithmetic and provenance]] — uses this mechanism as stated in its prerequisites.
- [[Call stack and stack frames]] — uses this mechanism as stated in its prerequisites.
- [[Dynamic allocation and heap]] — uses this mechanism as stated in its prerequisites.
- [[Static storage and initialization]] — uses this mechanism as stated in its prerequisites.
- [[Read-only data and code]] — uses this mechanism as stated in its prerequisites.
- [[Ownership and non-owning views]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Explain why returning a local pointer is invalid even if bytes remain.

**Pending:** Full lifetime replacement, ownership/concurrency policies and cross-target ABI experiments; the opening examples do not establish those later guarantees.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]] · [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Storage is a place; an object is its occupant|C007 development]]
