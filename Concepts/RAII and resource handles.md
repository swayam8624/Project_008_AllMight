---
tags:
  - concept
  - cpp
---

# RAII and resource handles

**Curriculum depth:** Introduced · **First seen:** C010 · **Latest development:** C010

## Definition and mechanism

Resource Acquisition Is Initialization (RAII) ties a cleanup responsibility to a well-defined object's lifetime. A resource handle acquires or adopts responsibility, preserves the release protocol through moves/composition, and performs matching cleanup on scope exit—including ordinary exception unwinding. The resource can be a memory allocation, lock, file, socket, native GPU object, or other paired protocol. RAII does not itself prove that an external asynchronous user has finished; it must trigger the correct deferred-release protocol.

Construction failure cleans completed subobjects, so each acquired responsibility should enter an owner immediately. A raw native handle is not an owner unless its release contract is implemented and documented.

## Prerequisites

- [[Storage duration and object lifetime]] — Object scope and resource lifetime must be distinguished.
- [[Destruction and partial construction]] — Completed owner subobjects clean up during failure.

## Used by

- [[Exclusive and shared smart-pointer ownership]]
- [[Placement construction and explicit lifetime management]]

## Recall and next depth

For a GPU buffer, identify the C++ owner, every borrower, the completion condition, and the native release call. Pending: allocator-aware and async-deferred reclamation protocols.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#From a live object to responsible resource ownership|M010 teaching narrative]] · [[Supplementary/Foundations#M010 - Parts 148-160|Source coverage]]
