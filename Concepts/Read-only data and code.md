---
tags:
  - concept
  - foundation
---

# Read-only data and code

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

Executable text commonly holds machine instructions; read-only mappings often hold literals/constants. W^X (writable or executable) policy discourages simultaneously writable executable storage, with platform/JIT qualifications. Modifying a string literal is undefined behavior regardless of whether hardware protection catches it. const automatic objects need not be in read-only mappings.

## Prerequisites

- [[Storage duration and object lifetime]] — Access requires an object and backing storage that are still valid.
- [[Process isolation and page permissions]] — The mapping context and permissions constrain which accesses proceed.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Separate C++ constness, literal lifetime, and page permissions.

**Pending:** Executable formats, loader protections, relocations and JIT transitions.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
