---
tags:
  - concept
  - cpp
---

# Null pointers and optional access

**Curriculum depth:** Introduced · **First seen:** C008 · **Latest development:** C008

## Definition and mechanism

A null pointer designates no object/function; nullptr_t converts to appropriate null pointer values without choosing an integer overload. A non-null pointer is not a lifetime or bounds proof. Deleting a typed null object pointer is valid, but bare delete nullptr is ill-formed.

## Prerequisites

- [[Pointer arithmetic and provenance]] — Storage relationships, bounds and lifetime distinguish valid access from a non-null value.

## Used by

Later sources will deepen this same node; no empty dependent nodes are created.

## Recall and next depth

Explain absence, stale access and one-past access separately. Pending: optional/reference wrapper APIs and owning-pointer policies. Written coverage does not establish learner mastery.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#A pointer carries an access contract|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]

## Absence must be part of the type or contract

A null pointer represents “no object at this address-bearing interface”; dereferencing it is invalid. That makes raw pointer nullability an API question. If a parameter must always designate an object, a reference or explicitly non-null wrapper communicates more than a nullable pointer plus a comment.

Optional ownership and optional observation are different. A `std::unique_ptr<T>` may be empty while also owning when nonempty; `T*` may be nullable but usually says nothing about ownership; `std::optional<T>` stores an optional value without indirection. Choosing among them should follow lifetime and absence semantics rather than syntax convenience.

Null checks also do not prove lifetime. A non-null dangling pointer is still invalid, and a pointer can become logically stale even if its numeric address is later reused for a different object.

Tests for optional APIs should cover absent state, present state, transitions between them, moved-from ownership and invalid external identifiers. Avoid sentinel pointer values other than the language-defined null representation unless a low-level ABI explicitly owns that convention.

