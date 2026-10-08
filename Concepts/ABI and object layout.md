---
tags:
  - concept
  - foundation
---

# ABI and object layout

**Curriculum depth:** Developed · **First seen:** C003 · **Latest development:** C007

## Definition and mechanism

C++23 preserves declaration order for nonzero-size non-variant data members even across different access labels; earlier standards had weaker ordering guarantees. alignas can request supported stronger alignment, increasing stride. Standard-layout and trivial copyability are distinct; offsetof is guaranteed for standard-layout types, while use on others is conditionally supported. Verify CPU/upload/shader offsets separately.

An application binary interface (ABI) specifies implementation-level calling and data-layout conventions. Member offsets, complete size/alignment, and calling conventions affect binary compatibility. sizeof/alignof/offsetof inspect a standard-layout record; they do not define a portable file schema. Assume stated target rules before deriving 0/4/8 offsets.

## Prerequisites

- [[Alignment and padding]] — Address divisibility determines legal placement and layout holes.
- [[Fixed-width integer types]] — The representation width fixes byte count and valid shift range.

## Used by

- [[Standard-layout and trivial copyability]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Separate measured compiler layout from a proposed wire schema.

**Pending:** Full lifetime replacement, ownership/concurrency policies and cross-target ABI experiments; the opening examples do not establish those later guarantees.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]] · [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Bringing the object model back to bytes|C007 development]]
