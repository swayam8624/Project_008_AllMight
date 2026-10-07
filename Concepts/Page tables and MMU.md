---
tags:
  - concept
  - foundation
---

# Page tables and MMU

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

The memory management unit (MMU) translates/protects accesses using mapping information. Page tables hold virtual-page→physical-frame mappings plus permissions/state, often hierarchically. A permitted VPN 12345→PFN 987 with offset ABC produces PA 987ABC. This model is architecture-neutral, not a specific entry format.

## Prerequisites

- [[Pages and page offsets]] — Page identity is translated while within-page position is preserved.
- [[Process isolation and page permissions]] — The mapping context and permissions constrain which accesses proceed.

## Used by

- [[TLB]] — uses this mechanism as stated in its prerequisites.
- [[Page faults and demand paging]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Explain what changes and what stays fixed in translation.

**Pending:** ARM/x86 table formats, walks, accessed/dirty bits, invalidation.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
