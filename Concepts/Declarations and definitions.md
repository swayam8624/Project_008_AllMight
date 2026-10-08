---
tags:
  - concept
  - cpp
---

# Declarations and definitions

**Curriculum depth:** Introduced · **First seen:** C007 · **Latest development:** C007

## Definition and mechanism

A declaration introduces or redeclares an entity; a definition provides the entity's defining information. extern int x is only a declaration; int x=0 defines an object; a function body defines a function. A forward-declared class is incomplete, but a fixed-underlying opaque enum is complete.

## Prerequisites

- [[Translation units and linking]] — A caller needs declaration/type information before the defining implementation is available.

## Used by

- [[Class access and invariants]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

- [[Aggregate initialization]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

- [[Scopes and name lookup]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

- [[Linkage and entity ownership]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

- [[One Definition Rule]] — uses this prerequisite to establish its entity, lifetime, representation or lookup contract.

Later chunks extend this same node; dependent concepts are listed here as they are integrated.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Promises become definitions|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
