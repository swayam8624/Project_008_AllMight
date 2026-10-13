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

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Read-only is a path, not an ownership or synchronization promise|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
