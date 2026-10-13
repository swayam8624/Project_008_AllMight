---
tags:
  - concept
  - cpp
---

# Move casts and forwarding references

**Curriculum depth:** Developed · **First seen:** C008 · **Latest development:** C009

## Definition and mechanism

Consumer constructors/assignments now distinguish the cast from actual resource transfer. Const can force copying, return std::move(local) can prevent NRVO, and move_if_noexcept selects a path based on usable construction properties.

std::move is an xvalue cast preserving constness, not a resource transfer. A deduced cv-unqualified T&& parameter can be a forwarding reference: lvalue arguments deduce T& and collapse to an lvalue reference. std::forward<T> restores the category encoded by deduction.

## Prerequisites

- [[Auto decltype and reference collapsing]] — Deduction and collapsing encode the caller category forwarding restores.
- [[Value categories and materialization]] — Expression category determines viable binding and whether temporary identity is needed.

## Used by

- [[Move operations and moved-from states]]

## Recall and next depth

Deduce both calls to relay and contrast unconditional move. Pending: deeper forwarding failures and constrained generic interfaces. Written coverage does not establish learner mastery.

## Source and navigation

[[Supplementary/Foundations#M009 - Parts 127-147|C009 development]]

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Move permission is not resource transfer|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
