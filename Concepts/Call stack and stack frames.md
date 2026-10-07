---
tags:
  - concept
  - foundation
---

# Call stack and stack frames

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

A call stack conventionally holds LIFO activation records: return state, saved registers, spills, some arguments/locals, and alignment. Optimized locals may use registers or disappear; automatic storage does not mandate a stack slot. Downward growth is platform convention. Capacity/guard mappings can limit recursion despite free RAM elsewhere.

## Prerequisites

- [[Storage duration and object lifetime]] — Access requires an object and backing storage that are still valid.
- [[Process isolation and page permissions]] — The mapping context and permissions constrain which accesses proceed.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Trace A→B→C→returns and identify the dead local.

**Pending:** Calling conventions, unwinding, coroutines, guard regions.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
