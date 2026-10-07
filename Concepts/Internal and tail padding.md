---
tags:
  - concept
  - foundation
---

# Internal and tail padding

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

Internal padding fills holes before members to satisfy alignment. Tail padding follows the final member so array stride preserves complete-object alignment. Under 1/4/1 rules, byte–uint32–byte occupies offsets 0/4/8 with end nine and size 12. Padding is not a semantic format field and cannot be relied on as stable encoded content.

## Prerequisites

- [[Alignment and padding]] — Address divisibility determines legal placement and layout holes.

## Used by

- [[Field reordering and AoS versus SoA]] — uses this mechanism as stated in its prerequisites.
- [[Packed structures and misaligned access]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Explain why stride nine breaks the second element's integer alignment.

**Pending:** Special member overlap, inheritance, and padding-safe comparisons.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
