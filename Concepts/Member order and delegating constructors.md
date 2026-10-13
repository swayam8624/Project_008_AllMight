---
tags:
  - concept
  - cpp
---

# Member order and delegating constructors

**Curriculum depth:** Introduced · **First seen:** C009 · **Latest development:** C009

## Definition and mechanism

Members initialize in declaration order, not initializer-list spelling order. A provider must precede a dependent member so reverse destruction cleans the dependent first. Delegation uses one target constructor as the sole initializer; the target completes before the delegating body.

## Prerequisites

- [[Constructors and invariant establishment]] — Initialization form and structural order determine how the constructor establishes members.

## Used by

- [[Destruction and partial construction]]

## Recall and next depth

Put a device and its dependent buffer in lifetime order; explain a delegating validation path. Pending: virtual bases and inheritance construction. Written coverage does not demonstrate personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Construction order records lifetime dependencies|Teaching mechanism and examples]] · [[Supplementary/Foundations#M009 - Parts 127-147|Exact coverage]] · [[Dependency Map]]
