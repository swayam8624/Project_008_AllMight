---
tags:
  - concept
  - cpp
---

# Parameter passing and sink contracts

**Curriculum depth:** Introduced · **First seen:** C009 · **Latest development:** C009

## Definition and mechanism

A by-value parameter is an object initialized from the argument expression: lvalues ordinarily copy, xvalues can move, and appropriate prvalues construct directly. References borrow; a pointer parameter copies its pointer value. A sink intentionally retains or consumes a value, not merely observes it.

## Prerequisites

- [[References and aliases]] — Parameter access differs according to whether a new object is created or an existing referent is borrowed.
- [[Const qualification and shallow constness]] — A read-only parameter path does not establish global immutability or ownership.

## Used by

Later sources will extend this node when their guarantees are developed.

## Recall and next depth

Compare lvalue and prvalue calls with spare capacity, then add growth work. Pending: generic forwarding interfaces, ABI measurements and allocator-specific costs. Written coverage does not demonstrate personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A value crosses the function boundary|Teaching mechanism and examples]] · [[Supplementary/Foundations#M009 - Parts 127-147|Exact coverage]] · [[Dependency Map]]
