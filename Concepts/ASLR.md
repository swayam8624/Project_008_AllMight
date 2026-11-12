---
tags:
  - concept
  - foundation
---

# ASLR

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

Address-space layout randomization varies mapping locations to make fixed-address assumptions less reliable and support exploitation resistance. It does not alter C++ member offsets or grant pointer validity. A pedagogical stack/heap/data/text ordering is not a guaranteed live map.

## Prerequisites

- [[Process isolation and page permissions]] — The mapping context and permissions constrain which accesses proceed.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Distinguish randomized mapping base from fixed member-relative offset.

**Pending:** Entropy, loader/ABI rules and platform security limits.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]

## Reconstruction depth

Address Space Layout Randomization changes where executable images, shared libraries, stacks, heaps and other mappings are placed between process runs. The security goal is to make an attacker's guessed code or data address less reliable; it is not memory encryption and it does not prevent a program from using valid pointers after the loader establishes mappings.

For engine code, the important consequence is that a virtual address is process-instance state, not durable identity. Serializing a raw pointer, hashing it as an asset identifier, or expecting the same module base across runs is invalid even before ASLR is considered; ASLR makes the mistake obvious.

Position-independent code and relocation machinery allow code and data references to work when the loader chooses a different base. Debuggers and crash reports therefore resolve addresses against the exact module build and load map rather than treating an absolute address as a universal source location.

A practical test is to run a tiny program multiple times and record stack, heap, function and shared-library addresses while keeping the binary unchanged. Values may vary, while relationships required by the program remain valid. ASLR strength and platform policy are operating-system evidence, not something portable C++ can infer from one run.

