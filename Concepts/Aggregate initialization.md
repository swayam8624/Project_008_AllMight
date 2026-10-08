---
tags:
  - concept
  - cpp
---

# Aggregate initialization

**Curriculum depth:** Introduced · **First seen:** C007 · **Latest development:** C007

## Definition and mechanism

C++20/23 aggregates include arrays and eligible classes without user-declared/inherited constructors or prohibited access/virtual features. Public member functions and default member initializers do not alone disqualify them. C++ designated initializers name direct members in declaration order; omitted members use applicable defaults or empty-list initialization.

## Prerequisites

- [[Declarations and definitions]] — The defined class's members and constructors determine whether aggregate element initialization applies.

## Used by

Later chunks extend this same node; dependent concepts are listed here as they are integrated.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Records acquire their initial state|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
