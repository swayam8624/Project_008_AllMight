---
tags:
  - concept
  - cpp
---

# Scopes and name lookup

**Curriculum depth:** Developed · **First seen:** C007 · **Latest development:** C008

## Definition and mechanism

Lookup precedes overload resolution. A derived overload can hide base overloads; using Base::name restores candidates. A local int x=x reads the newly declared uninitialized local, not an outer integer.

Scope locates a name's binding; lookup resolves a spelling to declarations. A local x can hide ::x without destroying it. A block-scope static has a local name but persistent storage. Qualified lookup names a namespace or class explicitly.

## Prerequisites

- [[Declarations and definitions]] — A declaration binds a name before scope and lookup can determine which binding is found.

## Used by

- [[Argument-dependent lookup]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Supplementary/Foundations#M008 - Parts 113-126|C008 development and corrections]]

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A spelling needs a place and a meaning|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
