---
tags:
  - concept
  - cpp
---

# Argument-dependent lookup

**Curriculum depth:** Introduced · **First seen:** C007 · **Latest development:** C007

## Definition and mechanism

For a suitable unqualified function call, argument types contribute associated namespaces/classes to candidate lookup. dot(a,b) can find geometry::dot for geometry::Pair arguments. using std::swap; swap(a,b) combines a fallback with custom ADL candidates. Some ordinary lookup results suppress ADL.

## Prerequisites

- [[Scopes and name lookup]] — Ordinary lookup provides candidates and suppression rules before arguments can add associated candidates.

## Used by

Later chunks extend this same node; dependent concepts are listed here as they are integrated.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A spelling needs a place and a meaning|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
