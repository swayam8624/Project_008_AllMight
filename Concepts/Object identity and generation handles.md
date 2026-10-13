---
tags:
  - concept
  - cpp
---

# Object identity and generation handles

**Curriculum depth:** Developed · **First seen:** C007 · **Latest development:** C008

## Definition and mechanism

Copied handles are distinct C++ objects with equal application identities. A validated handle does not certify a previous component borrow; assignment may preserve a C++ slot while changing logical ownership.

Object value, identity and address answer different questions. Equal values can belong to different objects; the same address can host successive lifetimes. A logical index/generation handle must be checked against the owning registry's bounds, occupancy and generation. Counter wrap and registry identity remain policy obligations.

## Prerequisites

- [[Object construction and storage reuse]] — Storage reuse creates successive occupants, motivating handles that record the intended generation.

## Used by

- [[Borrow invalidation and logical identity]]
- [[Copy semantics and resource identity]]

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Supplementary/Foundations#M008 - Parts 113-126|C008 development and corrections]]

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Values, occupants and addresses tell different stories|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
