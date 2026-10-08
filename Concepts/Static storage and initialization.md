---
tags:
  - concept
  - foundation
---

# Static storage and initialization

**Curriculum depth:** Developed · **First seen:** C003 · **Latest development:** C007

## Definition and mechanism

constinit requires static initialization without making an object const. Dynamic initialization across translation units requires ordering care; explicit owners or function-local statics can provide a clear dependency path. Local-static first initialization is synchronized since C++11, but later calls/mutations still require normal concurrency rules. Initialization and shutdown dependencies are distinct.

Namespace-scope/static objects have program-duration storage. Initialized writable objects commonly use data-like regions; zero-initialized objects use BSS-like zero-fill, avoiding literal zeros in the executable. Some objects require runtime dynamic initialization; cross-unit order is a distinct problem from storage allocation.

## Prerequisites

- [[Storage duration and object lifetime]] — Access requires an object and backing storage that are still valid.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Explain a large zero-initialized global without equally large file bytes.

**Pending:** Full lifetime replacement, ownership/concurrency policies and cross-target ABI experiments; the opening examples do not establish those later guarantees.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]] · [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A name does not decide how long storage lasts|C007 development]]
