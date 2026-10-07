---
tags:
  - concept
  - foundation
---

# Pages and page offsets

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

A page is a virtual address block; a frame is page-sized physical backing. With page size P=2^k, divide VA into VPN=floor(VA/P) and offset=VA mod P. Translation substitutes frame identity while preserving offset. The 4-KiB examples use k=12; actual supported sizes are platform-dependent.

## Prerequisites

- [[Addresses and virtual memory]] — Mappings give numeric locations their process-specific storage identity.
- [[Bit mask]] — A low-bit mask isolates the within-page offset.
- [[Shift operations]] — Shifts relocate a selected byte or address field to its significance position.

## Used by

- [[Page tables and MMU]] — uses this mechanism as stated in its prerequisites.
- [[Virtual and physical contiguity]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Split 0x12345ABC into VPN 12345 and offset ABC for 4 KiB.

**Pending:** Architecture page sizes, multilevel address fields, huge pages.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
