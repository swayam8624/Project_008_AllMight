---
tags:
  - concept
  - foundation
---

# Storage duration and object lifetime

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

Storage duration specifies how long storage is potentially available: automatic, static, thread, or dynamic. Object lifetime describes when a particular initialized object exists within it. Old bytes can remain after lifetime ends without authorizing access. const restricts mutation, not physical placement. Language categories do not dictate a universal segment map.

## Prerequisites

- [[Interpretation contract]] — Representation, type, and lifetime must identify what object exists.

## Used by

- [[Pointer arithmetic and provenance]] — uses this mechanism as stated in its prerequisites.
- [[Call stack and stack frames]] — uses this mechanism as stated in its prerequisites.
- [[Dynamic allocation and heap]] — uses this mechanism as stated in its prerequisites.
- [[Static storage and initialization]] — uses this mechanism as stated in its prerequisites.
- [[Read-only data and code]] — uses this mechanism as stated in its prerequisites.
- [[Ownership and non-owning views]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Explain why returning a local pointer is invalid even if bytes remain.

**Pending:** Construction/destruction, reuse, implicit-lifetime objects, thread-local storage.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
