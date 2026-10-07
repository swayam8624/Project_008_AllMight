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

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
