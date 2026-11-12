---
tags:
  - concept
  - cpp
---

# Multilevel pointer qualification

**Curriculum depth:** Introduced · **First seen:** C008 · **Latest development:** C008

## Definition and mechanism

int** cannot implicitly become const int**: that would permit injection of an actually const object's address into an original int*. int** can become const int* const*, whose intermediate read-only pointer blocks that assignment.

## Prerequisites

- [[Const qualification and shallow constness]] — Access-path qualification constrains nested conversions and member writes.

## Used by

Later sources will deepen this same node; no empty dependent nodes are created.

## Recall and next depth

Rebuild the hypothetical pointer injection step by step. Pending: qualification decomposition at further levels. Written coverage does not establish learner mastery.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Read-only is a path, not an ownership or synchronization promise|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]

## Const applies at each indirection level

For a pointer chain, qualification is attached to a particular level rather than to the whole expression. `const T*` means the pointed-to `T` is not writable through that path; `T* const` means the pointer object itself cannot be reseated; `const T* const` combines both.

The distinction becomes safety-critical at multiple levels. Converting `T**` to `const T**` is not generally safe: if it were allowed, code could store a pointer-to-const object into the outer pointer and later write through the original `T*`, violating const protection. Qualification conversions therefore follow stricter rules than the familiar `T* -> const T*` case.

Engine APIs should avoid raw multi-level pointer mutation unless the ownership/indirection contract truly requires it. Spans, references to containers, output wrappers or typed handle arrays often express the real intent more clearly. A reconstruction exercise should explain each const position and demonstrate the classic unsafe hypothetical conversion.

