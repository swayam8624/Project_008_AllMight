---
tags:
  - concept
  - cpp
---

# Special member generation and traits

**Curriculum depth:** Introduced · **First seen:** C009 · **Latest development:** C009

## Definition and mechanism

Declarations, deletion and expression usability differ. A user-declared destructor suppresses dedicated implicit moves even when defaulted. is_move_constructible asks if construction from T&& works, which can use copying; an explicitly deleted best move can still reject that expression.

## Prerequisites

- [[Copy semantics and resource identity]] — Assignment and moving must implement a defined semantic value rather than a raw pointer copy.
- [[Move operations and moved-from states]] — Traits, failure policy and relocation must distinguish source survival from source destruction.
- [[Default and value initialization]] — User-declared/user-provided distinctions affect generation and triviality as well as initial state.

## Used by

- [[Noexcept and transfer selection]]
- [[Rules of Three Five and Zero]]

## Recall and next depth

Explain the Suppressed probe and contrast explicit deletion with a defaulted-deleted move. Pending: union, constrained and inherited special-member rules. Written coverage does not demonstrate personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Let the compiler compose the operations|Teaching mechanism and examples]] · [[Supplementary/Foundations#M009 - Parts 127-147|Exact coverage]] · [[Dependency Map]]
