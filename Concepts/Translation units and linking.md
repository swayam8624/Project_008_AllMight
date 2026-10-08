---
tags:
  - concept
  - cpp
---

# Translation units and linking

**Curriculum depth:** Introduced · **First seen:** C007 · **Latest development:** C007

## Definition and mechanism

A translation unit is source after preprocessing together with its included material; module units add ownership and dependency rules. Compilation checks meaning and emits code/interface information; linking resolves implementation references. An unknown type is a frontend error, while a declared but undefined odr-used function commonly fails linking.

## Prerequisites

- [[Compiler IR and machine instructions]] — Compilation and emitted code explain why separately translated units need a link step.

## Used by

- [[Preprocessing and macro boundaries]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

- [[C++ modules and reachability]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

- [[Declarations and definitions]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

Later chunks extend this same node; dependent concepts are listed here as they are integrated.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Giving a program a buildable shape|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
