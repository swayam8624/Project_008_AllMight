---
tags:
  - concept
  - foundation
---

# Page faults and demand paging

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

A memory fault requires OS intervention when current translation/protection state cannot satisfy access. Valid demand loading, zero-fill, or copy-on-write may install/alter mapping then retry. Invalid addresses or denied permissions may fail instead. A fault need not perform disk I/O; a TLB miss need not fault at all.

## Prerequisites

- [[Page tables and MMU]] — Cached or faulting translation starts from the mapping/protection mechanism.

## Used by

- [[Copy-on-write]] — uses this mechanism as stated in its prerequisites.
- [[Memory-mapped files]] — uses this mechanism as stated in its prerequisites.

## Recall and next depth

Trace successful first-touch recovery versus denied access.

**Pending:** Minor/major faults, working sets, swapping, and platform terminology.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
