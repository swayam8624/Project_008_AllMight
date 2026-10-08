---
tags:
  - concept
  - foundation
---

# Pointer arithmetic and provenance

**Curriculum depth:** Developed · **First seen:** C003 · **Latest development:** C007

## Definition and mechanism

A scalar member behaves as a one-element array for pointer arithmetic, not as an array containing adjacent sibling members. &x+1 can be a one-past pointer but cannot be dereferenced to access y. Actual array storage supports traversal; a span only views an already-valid range. Layout/size assertions do not authorize sibling traversal.

A pointer is a typed access path constrained by array bounds, alignment, and object lifetime, not merely an integer. Within an array, p+i advances i elements; one-past is permitted for traversal but not dereference. Byte views inspect representations; uintptr_t, when provided, is an integer conversion facility, not permission to access arbitrary storage.

## Prerequisites

- [[Addresses and virtual memory]] — Mappings give numeric locations their process-specific storage identity.
- [[Storage duration and object lifetime]] — Access requires an object and backing storage that are still valid.

## Used by

- [[Named members versus array elements]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Explain p+1 for int versus byte and why a numeric address is insufficient.

**Pending:** Full lifetime replacement, ownership/concurrency policies and cross-target ABI experiments; the opening examples do not establish those later guarantees.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]] · [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Bringing the object model back to bytes|C007 development]]
