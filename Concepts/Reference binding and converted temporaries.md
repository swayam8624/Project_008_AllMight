---
tags:
  - concept
  - cpp
---

# Reference binding and converted temporaries

**Curriculum depth:** Introduced · **First seen:** C008 · **Latest development:** C008

## Definition and mechanism

Binding chooses an initial referent under type/category rules. A const int& bound to an int lvalue tracks that integer; a const double& initialized from the same int binds a converted temporary snapshot. Base-subobject binding need not copy; bit-fields cannot provide ordinary mutable integer references.

## Prerequisites

- [[References and aliases]] — The initial alias and its referent determine later binding and deduction behavior.
- [[Value categories and materialization]] — Expression category determines viable binding and whether temporary identity is needed.

## Used by

- [[Temporary lifetime extension]]

## Recall and next depth

Mutate the original after both bindings and predict their values. Pending: user-defined conversion overloads and volatile binding. Written coverage does not establish learner mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Binding chooses a referent, not always the one you imagined|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
