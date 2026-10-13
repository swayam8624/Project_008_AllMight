---
tags:
  - concept
  - cpp
---

# Ref-qualified accessors

**Curriculum depth:** Developed · **First seen:** C008 · **Latest development:** C009

## Definition and mechanism

Value-returning temporary accessors provide an owning result through direct construction or member movement. Both const and mutable rvalue policies remain required; the shorter M009 source example does not replace the four-way safe design.

Member &/&& qualifiers constrain the caller's object category. Borrowed getters suit persistent owners; temporary getters can return owning values or be deleted. Handle const rvalues too: a missing const&& policy can expose a borrowed result through const&. Inside && members, *this remains an lvalue.

## Prerequisites

- [[Const qualification and shallow constness]] — Access-path qualification constrains nested conversions and member writes.
- [[Value categories and materialization]] — Expression category determines viable binding and whether temporary identity is needed.
- [[Temporary lifetime extension]] — Temporary deadlines constrain returned paths and deferred access.

## Used by

Later sources will deepen this same node; no empty dependent nodes are created.

## Recall and next depth

Write all four Name overloads and justify each return type. Pending: explicit object parameters and generic interface deduction. Written coverage does not establish learner mastery.

## Source and navigation

[[Supplementary/Foundations#M009 - Parts 127-147|C009 development]]

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#An accessor must respect the caller's lifetime|Teaching explanation and examples]] · [[Supplementary/Foundations#M008 - Parts 113-126|Exact coverage]] · [[Dependency Map]]
