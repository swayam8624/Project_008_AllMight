---
tags:
  - concept
  - foundation
---

# Copy-on-write

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

COW initially shares physical contents while deferring private copies. A permitted write to a COW-protected mapping faults; OS allocates/copies a frame, updates the writer's mapping, and retries. Other mappings retain old contents. Not every read-only page is COW, so not every write fault is recoverable.

## Prerequisites

- [[Page faults and demand paging]] — Deferred private copying is implemented through a recoverable write fault.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Show frame identity before and after one process writes.

**Pending:** Fork behavior, mapping flags, reference tracking and costs.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]

## Reconstruction depth

Copy-on-write delays physical copying until shared data is actually modified. At the virtual-memory level, two address spaces may initially map the same physical page as read-only. A write triggers a protection fault; the operating system allocates a private page, copies old contents, updates the writer's page table and resumes the instruction.

The key invariant is logical independence after a write, even though physical storage was shared before it. This is why process creation can be cheap initially without eagerly duplicating every writable page.

COW has costs that disappear in a high-level description: the first write can incur a page fault and page copy, dirty-page accounting changes, and large working sets can turn an initially cheap clone into substantial later memory traffic. Huge pages and NUMA placement make those costs workload-dependent.

Do not generalize OS page COW into a recommendation for C++ value types. Library-level copy-on-write strings or containers complicate iterator/reference validity, threading and mutation semantics. The mechanism is useful when ownership and fault or copy behavior are explicit.

