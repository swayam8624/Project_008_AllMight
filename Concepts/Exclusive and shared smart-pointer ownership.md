---
tags:
  - concept
  - cpp
---

# Exclusive and shared smart-pointer ownership

**Curriculum depth:** Introduced · **First seen:** C010 · **Latest development:** C010

## Definition and mechanism

`std::unique_ptr<T, D>` is a move-only exclusive owner whose deleter `D` defines disposal. `get()` exposes a borrow; `release()` transfers the responsibility without deleting. `std::shared_ptr<T>` joins a shared ownership group represented by a control block; the managed object is destroyed when the last strong owner is released. The control block can outlive the object while weak observers remain. The stored pointer and the object whose destruction the control block arranges can differ (for example, aliasing ownership).

Shared ownership is not pointee synchronization. Concurrent operations on distinct smart-pointer handles do not automatically make concurrent access to `T` safe. Use sharing only when several independent parties truly retain the same lifetime.

## Prerequisites

- [[Ownership and non-owning views]] — Access-only views and ownership responsibilities are different contracts.
- [[RAII and resource handles]] — Smart pointers are standard resource owners built around lifetime-bound cleanup.

## Used by

- [[Weak observation and ownership cycles]]
- [[Placement construction and explicit lifetime management]]

## Recall and next depth

Explain why `unique_ptr<Derived>` converted to `unique_ptr<Base>` needs a valid base destruction policy. Pending: allocator/deleter state and atomic shared ownership interfaces.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Exclusive ownership: `unique_ptr` makes transfer visible|M010 teaching narrative]] · [[Supplementary/Foundations#M010 - Parts 148-160|Source coverage]]
