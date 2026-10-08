---
tags:
  - concept
  - cpp
---

# Class access and invariants

**Curriculum depth:** Introduced · **First seen:** C007 · **Latest development:** C007

## Definition and mechanism

struct and class differ in default member and base-class access: public versus private. Either supports methods, inheritance, constructors and hidden data. The record-versus-invariant distinction is design convention; an invariant is a condition each public operation must preserve.

## Prerequisites

- [[Declarations and definitions]] — Class declarations establish members and bases whose access must be public or controlled.

## Used by

Later chunks extend this same node; dependent concepts are listed here as they are integrated.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Public records and protected invariants|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
