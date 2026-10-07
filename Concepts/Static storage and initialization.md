---
tags:
  - concept
  - foundation
---

# Static storage and initialization

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

Namespace-scope/static objects have program-duration storage. Initialized writable objects commonly use data-like regions; zero-initialized objects use BSS-like zero-fill, avoiding literal zeros in the executable. Some objects require runtime dynamic initialization; cross-unit order is a distinct problem from storage allocation.

## Prerequisites

- [[Storage duration and object lifetime]] — Access requires an object and backing storage that are still valid.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Explain a large zero-initialized global without equally large file bytes.

**Pending:** Constant initialization, static initialization order, local-static guards.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
