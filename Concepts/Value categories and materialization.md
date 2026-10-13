---
tags:
  - concept
  - cpp
---

# Value categories and materialization

**Curriculum depth:** Developed · **First seen:** C008 · **Latest development:** C009

## Definition and mechanism

M009 develops result objects, direct same-type prvalue construction and C++23 move-eligible return expressions. Ordinary named-reference uses remain lvalues; optional NRVO is distinct from guaranteed direct construction.

lvalues and xvalues identify objects/functions; prvalues produce values/results. Glvalue contains lvalue/xvalue; rvalue contains prvalue/xvalue. A named rvalue-reference variable is an lvalue in ordinary use. Materialization supplies temporary identity when needed; it does not require a redundant copy of every class prvalue.

## Prerequisites

- [[Declarations and definitions]] — Declarations establish the entities and types whose uses are being classified.

## Used by

- [[Reference binding and converted temporaries]]
- [[Auto decltype and reference collapsing]]
- [[Move casts and forwarding references]]
- [[Ref-qualified accessors]]
- [[Return values and copy elision]]

## Recall and next depth

Classify rr, std::move(rr), ++i and i++. Pending: overlapping subobjects and detailed materialization/return deduction. Written coverage does not establish learner mastery.

## Source and navigation

[[Supplementary/Foundations#M009 - Parts 127-147|C009 development]]

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#An expression has a type and a category|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
