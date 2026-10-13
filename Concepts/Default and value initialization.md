---
tags:
  - concept
  - cpp
---

# Default and value initialization

**Curriculum depth:** Introduced · **First seen:** C009 · **Latest development:** C009

## Definition and mechanism

No scalar initializer can leave automatic integers indeterminate. Aggregate empty braces initialize omitted members, whereas a user-provided empty default constructor may leave its scalar member uninitialized even with braces. First-declaration defaulting differs from out-of-class defaulting.

## Prerequisites

- [[Constructors and invariant establishment]] — Initialization form and structural order determine how the constructor establishes members.

## Used by

- [[Special member generation and traits]]

## Recall and next depth

Classify int n, int n{}, defaulted A{}, and empty-body B{} without reading the bad cases. Pending: initializer-list overload selection and language-version uninitialized-value rules. Written coverage does not demonstrate personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Empty syntax does not promise initialized members|Teaching mechanism and examples]] · [[Supplementary/Foundations#M009 - Parts 127-147|Exact coverage]] · [[Dependency Map]]
