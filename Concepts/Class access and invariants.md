---
tags:
  - concept
  - cpp
---

# Class access and invariants

**Curriculum depth:** Developed · **First seen:** C007 · **Latest development:** C009

## Definition and mechanism

Constructors establish externally usable state before publication; failed ordinary construction destroys completed subobjects. Generated memberwise moves do not automatically preserve relations such as extent-to-buffer consistency.

struct and class differ in default member and base-class access: public versus private. Either supports methods, inheritance, constructors and hidden data. The record-versus-invariant distinction is design convention; an invariant is a condition each public operation must preserve.

## Prerequisites

- [[Declarations and definitions]] — Class declarations establish members and bases whose access must be public or controlled.

## Used by

- [[Logical constness and mutable caches]]
- [[Constructors and invariant establishment]]

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** polymorphic invariants and subsystem-level failure policies.

## Source and navigation

[[Supplementary/Foundations#M009 - Parts 127-147|C009 development]]

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Public records and protected invariants|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
