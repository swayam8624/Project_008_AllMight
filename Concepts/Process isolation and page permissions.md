---
tags:
  - concept
  - foundation
---

# Process isolation and page permissions

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

An address-space mapping context determines which physical frame a virtual number names and whether read/write/execute access is permitted. Two processes may use VA 0x400000 for different frames; shared mappings can deliberately use one frame. Permission checks belong to mapping state, not to the apparent numerical validity of a pointer.

## Prerequisites

- [[Addresses and virtual memory]] — Mappings give numeric locations their process-specific storage identity.

## Used by

- [[Page tables and MMU]] — uses this mechanism as stated in its prerequisites.
- [[Call stack and stack frames]] — uses this mechanism as stated in its prerequisites.
- [[Read-only data and code]] — uses this mechanism as stated in its prerequisites.
- [[ASLR]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Give private and deliberately shared mapping examples.

**Pending:** Kernel/user isolation and architecture-specific protection.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
