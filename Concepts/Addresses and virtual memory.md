---
tags:
  - concept
  - foundation
---

# Addresses and virtual memory

**Curriculum depth:** Developed · **First seen:** C002 · **Latest development:** C003

## Definition and mechanism

An address identifies a location in an address space. User-space object addresses are normally virtual, translated through per-process mappings to physical frames with permissions. A page translation preserves offset; VA 12345ABC mapping VPN 12345 to PFN 987 yields PA 987ABC with 4-KiB pages. Adjacent virtual pages may have scattered physical backing. Numeric address, object lifetime, and access legality are separate.

## Prerequisites

- [[Byte and octet]] — Byte units define both storage offsets and significance digits.

## Used by

- [[Pointer arithmetic and provenance]] — uses this mechanism as stated in its prerequisites.
- [[Pages and page offsets]] — uses this mechanism as stated in its prerequisites.
- [[Process isolation and page permissions]] — uses this mechanism as stated in its prerequisites.
- [[Alignment and padding]] — uses this mechanism as stated in its prerequisites.

- [[Cache lines and memory bandwidth]] — depends on this concept; see its prerequisite explanation.

## Recall and next depth

Follow member offset to VA, page decomposition, permissions, and physical byte.

**Pending:** Architecture translation formats, residency and memory hierarchy.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
