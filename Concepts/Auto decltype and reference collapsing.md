---
tags:
  - concept
  - cpp
---

# Auto decltype and reference collapsing

**Curriculum depth:** Introduced · **First seen:** C008 · **Latest development:** C008

## Definition and mechanism

Ordinary by-value auto drops top-level const/reference information; auto& preserves referent qualification. decltype(name) reports the declared type, while decltype((expression)) follows category. Collapsing yields an lvalue reference if either participating reference is &, otherwise &&.

## Prerequisites

- [[Value categories and materialization]] — Expression category determines viable binding and whether temporary identity is needed.
- [[References and aliases]] — The initial alias and its referent determine later binding and deduction behavior.

## Used by

- [[Move casts and forwarding references]]

## Recall and next depth

Explain why decltype(rr) is T&& but decltype((rr)) is T&. Pending: constrained deduction, structured bindings and return-type deduction. Written coverage does not establish learner mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#An expression has a type and a category|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
