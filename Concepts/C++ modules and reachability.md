---
tags:
  - concept
  - cpp
---

# C++ modules and reachability

**Curriculum depth:** Introduced · **First seen:** C007 · **Latest development:** C007

## Definition and mechanism

A named module imports semantic interfaces rather than textually pasting headers. A BMI supplies compiler-facing interface information; object code still participates in linking. Export visibility is not identical to semantic reachability, and a named-module import is distinct from a macro-bearing header-unit import.

## Prerequisites

- [[Translation units and linking]] — Interface information serves compilation, while object code serves linking; modules change their distribution.

## Used by

Later chunks extend this same node; dependent concepts are listed here as they are integrated.

## Recall and next depth

Give an example, explain why the rule exists, and identify which guarantee would be missing if the prerequisite were ignored.

**Pending:** Deeper ownership, pointer/reference, generic programming and module rules from later sources. This is written coverage, not demonstrated personal mastery.

## Source and navigation

[[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Giving a program a buildable shape|C007 teaching chapter]] · [[Supplementary/Foundations#M007 - Parts 101-112|Source coverage]] · [[Dependency Map]]
